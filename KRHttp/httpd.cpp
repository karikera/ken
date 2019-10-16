#include <KR3/main.h>

#include "httpd.h"

#include <KR3/fs/file.h>
#include <KR3/initializer.h>
#include <KR3/mt/thread.h>
#include <KR3/wl/windows.h>

using namespace kr;

inline AText readWithHeader(Text header, pcstr16 path)
{
	Must<File> file = File::open(path);
	dword filesize = file->size32();
	size_t headersize = header.size();
	AText contents(headersize + filesize);
	contents.subcopy(header.begin(), headersize, 0);
	file->readImpl(contents.begin() + headersize, filesize);
	return contents;
}

constexpr size_t HEADER_LENGTH_LIMIT = 4096;
static size_t clientCount = 0;
static CriticalSection doutLock;

HttpClient::HttpClient(HttpServer * server, Socket * socket) noexcept
	:MTClient(socket)
{
	m_state = ReadHeader;
	m_server = server;

	CsLock __lock = doutLock;
	clientCount++;
	dout << "HTTP Client Count: " << clientCount << endl;
}
HttpClient::~HttpClient() noexcept
{
	m_get.clear();
	m_post.clear();
	m_header.clear();

	CsLock __lock = doutLock;
	clientCount--;
	dout << "Client is closed" << endl;
	dout << "HTTP Client Count: " << clientCount << endl;
}
void HttpClient::onRead() throws(...)
{
	for (;;)
	{
		switch (m_state)
		{
		case ReadHeader:
			try
			{
				{
					TText headerBuffer;
					Text header = m_receive.readwith("\r\n\r\n", &headerBuffer);
					if (header == nullptr)
					{
						if (m_headerData.size() >= HEADER_LENGTH_LIMIT) return;
						size_t size = m_receive.size() - 3;
						m_receive.read(m_headerData.prepare(size), size);
						return;
					}
					m_headerData << header;
				}

				// parse headline
				Text header = m_headerData;
				Text headline = header.readwith("\r\n");
				if (headline == nullptr) return;

				m_method = headline.readwith(' ');
				if (m_method == "POST")
				{
				}
				else if (m_method == "GET")
				{
				}
				else
				{
					throw HttpMethodNotImplemented;
				}

				// parse headers
				m_header.set(header);

				// process path
				m_uriRequest = headline.readwith(' ');
				Text url = m_uriRequest;

				if (url.empty()) throw HttpBadRequest;
				if (url.front() != '/') throw HttpBadRequest;
				url++;
				Text query = url.find('?');
				if (query != nullptr)
				{
					url = url.cut(query);
					query++;
				}
				m_path = url;
				m_query = query;

				m_server->findPage(m_path, &m_fp);

				if (m_fp.page)
				{
					m_state = ProcessPage;
				}
				else
				{
					m_state = SendFile;
				}

			}
			catch (HttpStatus status)
			{
				m_fp.page = m_server->getErrorPage(status);
				m_state = ProcessPage;
			}
			break;
		case ProcessPage:
			try
			{
				m_fp.page->process(this, m_query, &m_receive);
				m_state = IgnoreReceive;
				return;
			}
			catch (HttpStatus code)
			{
				m_fp.page = m_server->getErrorPage(code);
				m_state = ProcessErrorPage;
			}
			catch (...)
			{
				m_fp.page = m_server->getErrorPage(HttpServerInternalError);
				m_state = ProcessErrorPage;
			}
			break;
		case ProcessErrorPage:
			try
			{
				m_fp.page->process(this, m_query, &m_receive);
				m_state = IgnoreReceive;
			}
			catch (...)
			{
				close();
			}
			return;
		case SendFile:
		{
			TmpArray<char> header((size_t)0, 1024);
			header <<
				"HTTP/1.0 200 OK\r\n"
				"MIME-Version: 1.0\r\n"
				"Content-Type:";
			header << m_server->getMIMEType(m_fp.path);
			header << "\r\n";
			header << m_server->getDefaultHeader();
			write(header.cast<void>());
			m_state = IgnoreReceive;
			onSendDone();
			return;
		}
		case IgnoreReceive:
			return;
		}
	}
}
void HttpClient::onSendDone() noexcept
{
	if (m_fp.file)
	{
		try
		{
			TmpArray<char> buffer(8192);
			buffer.resize(m_fp.file->readImpl(buffer.begin(), buffer.size()));
			write(buffer.cast<void>());
			flush();
		}
		catch (EofException&)
		{
			m_fp.file = nullptr;
			close();
		}
	}
}
void HttpClient::onError(Text funcname, int code) noexcept
{
	CsLock __lock = doutLock;
	dout << "Client error: " << funcname << " (code: " << code << ")" << endl;
}
void HttpClient::readPostVariable(Text * stream)
{
}

void kr::HttpClient::parseQueryTo(ReferenceMap<Text, Text>* target, Text data) noexcept
{
	for (;;)
	{
		Text name = data.readwith('=');
		if (name == nullptr) return;

		Text to = data;
		for (;;)
		{
			to = to.find('&');
			if (to == nullptr)
			{
				(*target)[name] = data;
				return;
			}
			else if ((to + 1).startsWith("amp;"))
			{
				to += 5;
				continue;
			}

			Text value = data.readto_p(to);
			(*target)[name] = value;
			data++;
			break;
		}
	}
}
Text HttpClient::getHeader(Text name) noexcept
{
	return m_header[name];
}
Text HttpClient::get(Text name) noexcept
{
	return getMap()[name];
}
Text HttpClient::post(Text name) noexcept
{
	return postMap()[name];
}
HttpHeader& HttpClient::getHeader() noexcept
{
	return m_header;
}
ReferenceMap<Text, Text>& HttpClient::getMap() noexcept
{
	if (!m_getParsed)
	{
		m_getParsed = true;
		parseQueryTo(&m_get, m_query);
	}
	return m_get;
}
ReferenceMap<Text, Text>& HttpClient::postMap() noexcept
{
	if (!m_postParsed)
	{
		m_postParsed = true;
		parseQueryTo(&m_post, m_query);
	}
	return m_post;
}

HttpServer::HttpServer() noexcept
{
	m_mime["html"] = "text/html";
	m_mime["js"] = "text/javascript";
	m_error[HttpNotModified] =  _new StaticPage("HTTP/1.0 304 Not Modified\r\n\r\n");
	m_error[HttpMethodNotImplemented] =  _new StaticPage("HTTP/1.0 501 Method Not Implemented\r\nContent-Type: text/plain\r\n\r\nNot implemented");
	m_error[HttpNotFound] = _new StaticPage("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found");
	m_headers = "\r\n";
}
HttpServer::~HttpServer() noexcept
{
	{
		CsLock __lock = doutLock;
		dout << "Server is closed" << endl;
	}
	for(auto &iter : m_error)
	{
		delete iter.second;
	}
	for (auto &iter : m_map)
	{
		delete iter.second;
	}
}
void HttpServer::setDefaultHeader(pcstr16 filename) noexcept
{
	m_headers = File::openAsArray<char>(filename);
	if (m_headers.size() < 4)
	{
		m_headers = "\r\n\r\n";
	}
	else
	{
		if (m_headers.subarr(m_headers.size() - 2, 2) == "\r\n")
		{
			if (m_headers.subarr(m_headers.size() - 4, 2) == "\r\n")
			{
			}
			else
			{
				m_headers << "\r\n";
			}
		}
		else
		{
			m_headers << "\r\n\r\n";
		}
	}
}
Text HttpServer::getDefaultHeader() noexcept
{
	return m_headers;
}
void HttpServer::setErrorPage(HttpStatus exception, pcstr16 filename) noexcept
{
	MemoryPage * fl = _new MemoryPage(filename);
	auto res = m_error.insert({exception, fl});
	if (res.second) return;
	delete res.first->second;
	res.first->second = fl;
}
void HttpServer::setTemplatePage(Text url, pcstr16 filename) noexcept
{
	attachPage(url, _new TemplatePage(filename));
}
void HttpServer::setMIMEType(Text type, Text mime) noexcept
{
	m_mime[type] = mime;
}
Text HttpServer::getMIMEType(Text fileName) noexcept
{
	Text ext = fileName.find_r('.');
	if (ext == nullptr)
	{
		return "application/octet-stream";
	}
	else
	{
		auto iter2 = m_mime.find(ext + 1);
		if (iter2 == m_mime.end())
		{
			return "application/octet-stream";
		}
		else
		{
			return iter2->second;
		}
	}
}
void HttpServer::attachPage(Text url, Page * page) noexcept
{
	auto res = m_map.insert( url, page );
	if (res.second) return;
	delete res.first->second;
	res.first->second = page;
}
void HttpServer::findPage(Text path, HttpFindPage * fp) noexcept
{
	HttpStatus exception;
	try
	{
		// simplize path
		AText nurl((size_t)0, path.size() + 32);
		for (Text name : path.splitIterable('/'))
		{
			if (name == "..")
			{
				Text rev = name.find_r('/');
				if (rev == nullptr) throw HttpForbidden;
				nurl.cut_self(rev);
			}
			nurl << '/' << name;
		}

		// get memory page
		auto iter = m_map.find(nurl);
		if (iter != m_map.end())
		{
			fp->page = iter->second;
			return;
		}

		// find file page
		if (nurl.back() == '/')
		{
			nurl << "index.html";
		}
		path = nurl + 1;

		TText16 file16;
		file16.prints(Utf8ToUtf16(path), nullterm);
		
		// unicode read
		fp->page = nullptr;
		fp->path = move(nurl);
		fp->file = File::open(file16.data());
		return;
	}
	catch (EofException&) { return; }
	catch (SocketException&) { return; }
	catch (Error&) { exception = HttpNotFound; }
	catch (TooBigException&) { exception = HttpServerInternalError; }
	catch (HttpStatus e) { exception = e; }

	Page * page;
	auto res = m_error.find(exception);
	if (res != m_error.end()) page = res->second;
	else page = m_error[HttpMethodNotImplemented];

	fp->file = nullptr;
	fp->page = page;
}
Page* HttpServer::getErrorPage(HttpStatus code) noexcept
{
	Page * page;
	auto res = m_error.find(code);
	if (res != m_error.end()) page = res->second;
	else page = m_error[HttpMethodNotImplemented];
	return page;
}

MTClient* HttpServer::onAccept(Socket * socket) noexcept
{
	return _new HttpClient(this, socket);
}
void HttpServer::onError(Text funcname, int error) noexcept
{
	CsLock __lock = doutLock;
	dout << "Server error: " << funcname << " (code: " <<  error << ")" << endl;
}

Page::Page() noexcept
{
}
Page::~Page() noexcept
{
}

StaticPage::StaticPage(Text text) noexcept
	:m_contents(text)
{
}
void StaticPage::process(HttpClient * client, Text query, BufferQueue * stream)
{
	client->write(m_contents.cast<void>());
	client->flush();
	client->close();
}

kr::TemplatePage::TemplatePage(pcstr16 filename) noexcept
{
	m_contents = File::openAsArray<char>(filename);
	Text source = m_contents;
	m_splitContents.reserve(50);
	m_keys.reserve(10);
	for (size_t i = 0;; i++)
	{
		Text find = source.readwith("{{");
		if (find == nullptr)
		{
			m_splitContents.push(source);
			break;
		}
		else
		{
			m_splitContents.push(find);
		}
		Text var = source.readwith("}}");
		if (var == nullptr) break;
		m_keys.insert({ std::string(var.begin(), var.size()), i });
	}
	m_splitContents.shrink();
}
kr::TemplatePage::~TemplatePage() noexcept
{
}
void kr::TemplatePage::parseQuery(Array<Text> &arr, Text prefix, Text query) noexcept
{
	for (;;)
	{
		Text name = query.readwith('=');
		if (name == nullptr) return;
		auto range = m_keys.equal_range(std::string(prefix.begin(), prefix.size()) + std::string(name.begin(), name.size()));
		Text to = query;
		for (;;)
		{
			to = to.find('&');
			if (to == nullptr)
			{
				for (auto i = range.first; i != range.second; i++)
				{
					arr[i->second] = query;
				}
				return;
			}
			else if (to + 1 == "amp;"_tx)
			{
				to += 5;
				continue;
			}

			Text value = query.readto_p(to);
			for (auto i = range.first; i != range.second; i++)
			{
				arr[i->second] = value;
			}
			query++;
			break;
		}
	}
}
void kr::TemplatePage::process(HttpClient * client, Text query, BufferQueue * stream)
{
	Array<Text> variables(m_keys.size());
	parseQuery(variables, "get:"_tx, query);
	AText postdata = stream->readAll();
	parseQuery(variables, "post:"_tx, postdata);
	size_t n = m_splitContents.size()-1;
	size_t i = 0;
	for (;i<n;i++)
	{
		client->write(m_splitContents[i].cast<void>());
		client->write(variables[i].cast<void>());
	}
	client->write(m_splitContents[i].cast<void>());
	client->flush();
	client->close();
}

MemoryPage::MemoryPage() noexcept
{
}
MemoryPage::MemoryPage(pcstr16 path) throws(Error, TooBigException)
	: m_contents(File::openAsArray<char>(path))
{
}
MemoryPage::MemoryPage(Text header, pcstr16 path) throws(Error)
{
	m_contents = nullptr;
	m_contents = readWithHeader(header, path);
}
MemoryPage::MemoryPage(const MemoryPage& _copy) noexcept
{
	m_contents = _copy.m_contents;
}
MemoryPage::MemoryPage(MemoryPage&& _move) noexcept
{
	m_contents = move(_move.m_contents);
}
MemoryPage::~MemoryPage() noexcept
{
}
MemoryPage & MemoryPage::operator =(const MemoryPage& _copy) noexcept
{
	this->~MemoryPage();
	new(this) MemoryPage(_copy);
	return *this;
}
MemoryPage & MemoryPage::operator =(MemoryPage&& _move) noexcept
{
	this->~MemoryPage();
	new(this) MemoryPage(move(_move));
	return *this;
}
void MemoryPage::process(HttpClient * client, Text query, BufferQueue * stream)
{
	client->write(m_contents.cast<void>());
	client->flush();
	client->close();
}
