#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else

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
	file->$read(contents.begin() + headersize, filesize);
	return contents;
}

constexpr size_t HEADER_LENGTH_LIMIT = 4096;
constexpr size_t POST_DATA_LENGTH_LIMIT = 4096;
constexpr size_t MULTIFORM_POST_DATA_LENGTH_LIMIT = 4096;
static size_t clientCount = 0;
static CriticalSection doutLock;

UriFormData::UriFormData(Text data) noexcept
{
	for (;;)
	{
		Text name = data.readwith('=');
		if (name == nullptr) return;

		Text to = data;
		for (;;)
		{
			to.subarr_self(to.find('&'));
			if (to == nullptr)
			{
				(*this)[name] = data;
				return;
			}
			else if ((to + 1).startsWith("amp;"))
			{
				to += 5;
				continue;
			}

			Text value = data.readto_p(to.data());
			(*this)[name] = value;
			data++;
			break;
		}
	}
}

MultipartFormData::MultipartFormData() noexcept
{
	m_boundary = nullptr;
	m_state = State::SkipBoundary;
}

void MultipartFormData::readHeader(HttpClient* client) throws(ThrowRetry, HttpStatus)
{
	if (m_boundary != nullptr) return;
	
	auto& headers = client->getHeader();

	Text contentLength = headers["Content-Length"];
	if (contentLength == nullptr) throw HttpStatus::MethodNotAllowed;
	m_contentSize = contentLength.to_uintp();

	Text contentType = headers["Content-Type"];
	if (contentType == nullptr) throw HttpStatus::MethodNotAllowed;
	if (contentType.readwith(';') != "multipart/form-data") throw HttpStatus::MethodNotAllowed;

	while (!contentType.empty())
	{
		if (*contentType == ' ') contentType++;
		Text varline = contentType.readwith_e(';');
		Text varname = varline.readwith('=');
		if (varname == nullptr) continue;
		if (varname == "boundary")
		{
			m_boundary = varline;
			m_boundary << "\r\n";
			return;
		}
	}
	throw HttpStatus::MethodNotAllowed;
}
void MultipartFormData::read(HttpClient* client, BufferQueue* receive) throws(ThrowRetry, HttpStatus)
{
	readHeader(client);
	HashTester<void> boundary(m_boundary.cast<void>(), HashTester<void>::NoReset);
	HashTester<void> lineend("\r\n"_tx.cast<void>(), HashTester<void>::NoReset);
	for (;;)
	{
		switch (m_state)
		{
		case State::SkipBoundary:
		{
			boundary.reset();
			BufferQueue::ReadResult res = receive->readto(boundary);
			size_t junksize = res.totalSize + boundary.size();
			m_contentSize -= junksize;
			receive->skip(junksize);
			if (!res.ended) throw ThrowRetry();
			m_state = State::Header;
			break;
		}
		case State::Header:
		{
			for (;;)
			{
				lineend.reset();
				BufferQueue::ReadResult res = receive->readto(lineend);
				res.readAllTo((ABuffer*)&m_currentHeader, MULTIFORM_POST_DATA_LENGTH_LIMIT);
				receive->skip(lineend.size());

				if (m_currentHeader.empty()) break;
				m_currentHeader = nullptr;
			}
			m_currentHeader = nullptr;
			m_state = State::Data;
			break;
		}
		case State::Data:
			m_state = State::Header;
			break;
		}
	}
}
Text MultipartFormData::get(Text name) noexcept
{
	return "";
}

HttpClient::HttpClient(HttpServer* server, Socket* socket) noexcept
	:MTClient(socket)
{
	m_state = State::ReadHeader;
	m_server = server;

	CsLock __lock = doutLock;
	clientCount++;
	dout << "HTTP Client Count: " << clientCount << endl;

	m_headerParsed = false;
}
HttpClient::~HttpClient() noexcept
{
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
		case State::ReadHeader:
			try
			{
				_readHeadLine();

				// parse headline
				Text headline = m_headLine;
				m_method = headline.readwith(' ');
				if (m_method == "POST")
				{
				}
				else if (m_method == "GET")
				{
				}
				else
				{
					throw HttpStatus::NotImplemented;
				}

				// process path
				m_uriRequest = headline.readwith(' ');
				Text url = m_uriRequest;
				if (url.empty()) throw HttpStatus::BadRequest;
				if (url.front() != '/') throw HttpStatus::BadRequest;
				url++;
				const char* query = url.find('?');
				if (query != nullptr)
				{
					url = url.cut(query);
					query++;
				}
				m_path = url;
				m_query = url.subarr(query);

				m_server->findPage(m_path, &m_fp);
				if (m_fp.page)
				{
					m_state = State::ProcessPage;
				}
				else
				{
					m_state = State::SendFile;
				}
			}
			catch (ThrowRetry)
			{
				return;
			}
			catch (NotEnoughSpaceException&)
			{
				m_fp.page = m_server->getErrorPage(HttpStatus::BadRequest);
				m_state = State::ProcessErrorPage;
			}
			catch (HttpStatus status)
			{
				m_fp.page = m_server->getErrorPage(status);
				m_state = State::ProcessPage;
			}
			break;
		case State::ProcessPage:
			try
			{
				m_fp.page->process(this);
				m_state = State::IgnoreReceive;
				return;
			}
			catch (ThrowRetry)
			{
				return;
			}
			catch (NotEnoughSpaceException&)
			{
				m_fp.page = m_server->getErrorPage(HttpStatus::BadRequest);
				m_state = State::ProcessErrorPage;
			}
			catch (HttpStatus code)
			{
				m_fp.page = m_server->getErrorPage(code);
				m_state = State::ProcessErrorPage;
			}
			catch (...)
			{
				m_fp.page = m_server->getErrorPage(HttpStatus::InternalServerError);
				m_state = State::ProcessErrorPage;
			}
			break;
		case State::ProcessErrorPage:
			try
			{
				m_fp.page->process(this);
				m_state = State::IgnoreReceive;
			}
			catch (...)
			{
				close();
			}
			return;
		case State::SendFile:
		{
			writeHeader({
				"HTTP/1.0 200 OK\r\n"
				"MIME-Version: 1.0\r\n"
				"Content-Type: ", m_server->getMIMEType(m_fp.path), "\r\n"
				"Content-Length: ", TSZ() << m_fp.file->size(), "\r\n"
				});
			m_state = State::IgnoreReceive;
			onSendDone();
			return;
		}
		case State::IgnoreReceive:
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
			buffer.resize(m_fp.file->$read(buffer.begin(), buffer.size()));
			write(buffer);
			flush();
		}
		catch (EofException&)
		{
			m_fp.file = nullptr;
			close();
		}
	}
	void writeHeader(View<Text> headers) noexcept;
	void writeHeader(View<Text> headers) noexcept;
}
void HttpClient::write(Text data) noexcept
{
	MTClient::write(data.cast<void>());
}
void HttpClient::writes(View<Text> data) noexcept
{
	MTClient::writes((View<Buffer>&)data);
}
void HttpClient::writeWithoutLock(Text data) noexcept
{
	MTClient::writeWithoutLock(data.cast<void>());
}
void HttpClient::writeHeader(View<Text> headers) noexcept
{
	writeLock();
	writeWithoutLock("HTTP/1.0 200 OK\r\n");
	for (Text line : headers)
	{
		writeWithoutLock(line);
	}
	writeWithoutLock(m_server->getDefaultHeader());
	writeUnlock();
}
void HttpClient::writeHeader() noexcept
{
	writeLock();
	writeWithoutLock("HTTP/1.0 200 OK\r\n"_tx);
	writeWithoutLock(m_server->getDefaultHeader());
	writeUnlock();
}
void HttpClient::onError(Text funcname, int code) noexcept
{
	CsLock __lock = doutLock;
	dout << "Client error: " << funcname << " (code: " << code << ")" << endl;
}
Text HttpClient::getMethod() noexcept
{
	return m_method;
}
Text HttpClient::getPath() noexcept
{
	return m_path;
}
Text HttpClient::getQuery() noexcept
{
	return m_query;
}
AText HttpClient::getPostData() throws(ThrowRetry, HttpStatus)
{
	Text value = getHeader("Content-Length");
	if (value == nullptr) return nullptr;

	size_t size = m_receive.size();
	size_t contentLength = value.to_uintp();
	if (contentLength >= POST_DATA_LENGTH_LIMIT) throw NotEnoughSpaceException();
	if (size < contentLength) throw ThrowRetry{};
	if (size >= POST_DATA_LENGTH_LIMIT) throw NotEnoughSpaceException();

	ABuffer out;
	for (Buffer buf : m_receive)
	{
		out << buf;
	}
	return move((AText&)out);
}
MultipartFormData& HttpClient::getMultipartFormData() throws(ThrowRetry, NotEnoughSpaceException)
{
	m_multipart.read(this, &m_receive);
	return m_multipart;
}

Text HttpClient::getHeader(Text name) throws(ThrowRetry, NotEnoughSpaceException)
{
	return getHeader()[name];
}
HttpHeader& HttpClient::getHeader() throws(ThrowRetry, NotEnoughSpaceException)
{
	if (!m_headerParsed)
	{
		HashTester<void> needle = "\r\n\r\n"_tx.cast<void>();
		m_receive.readto(needle).readAllTo((ABuffer*)&m_headerBuffers, HEADER_LENGTH_LIMIT);
		m_receive.skip(needle.size());
		m_header.set(m_headerBuffers);
		m_headerParsed = true;
	}

	return m_header;
}

void HttpClient::_readHeadLine() throws(ThrowRetry, NotEnoughSpaceException)
{
	HashTester<void> needle = "\r\n"_tx.cast<void>();
	m_receive.readto(needle).readAllTo((ABuffer*)&m_headLine, HEADER_LENGTH_LIMIT);
	m_receive.skip(needle.size());
}

HttpServer::HttpServer(AText16 htmlRoot) throws(FunctionError)
	:m_htmlRoot(move(htmlRoot))
{
	if (m_htmlRoot.endsWith_y(u"\\/")) m_htmlRoot.pop();

	m_mime["html"] = "text/html";
	m_mime["js"] = "text/javascript";
	m_error[HttpStatus::NotModified] =  _new StaticPage("HTTP/1.0 304 Not Modified\r\n\r\n");
	m_error[HttpStatus::NotImplemented] =  _new StaticPage("HTTP/1.0 501 Not Implemented\r\nContent-Type: text/plain\r\n\r\nNot implemented");
	m_error[HttpStatus::NotFound] = _new StaticPage("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found");
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
	pcstr ext = fileName.find_r('.');
	if (ext == nullptr)
	{
		return "application/octet-stream";
	}
	else
	{
		auto iter2 = m_mime.find(fileName.subarr(ext + 1));
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
				pcstr rev = name.find_r('/');
				if (rev == nullptr) throw HttpStatus::Forbidden;
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
		TText16 file16;
		file16.prints(m_htmlRoot, Utf8ToUtf16(nurl), nullterm);
		// unicode read
		fp->page = nullptr;
		fp->path = move(nurl);
		fp->file = File::open(file16.data());
		return;
	}
	catch (EofException&) { return; }
	catch (SocketException&) { return; }
	catch (Error&) { exception = HttpStatus::NotFound; }
	catch (TooBigException&) { exception = HttpStatus::InternalServerError; }
	catch (HttpStatus e) { exception = e; }

	Page * page;
	auto res = m_error.find(exception);
	if (res != m_error.end()) page = res->second;
	else page = m_error[HttpStatus::NotImplemented];

	fp->file = nullptr;
	fp->page = page;
}
Page* HttpServer::getErrorPage(HttpStatus code) noexcept
{
	Page * page;
	auto res = m_error.find(code);
	if (res != m_error.end()) page = res->second;
	else page = m_error[HttpStatus::NotImplemented];
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
void StaticPage::process(HttpClient * client)
{
	client->write(m_contents);
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
			to.subarr(to.find('&'));
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

			Text value = query.readto_p(to.data());
			for (auto i = range.first; i != range.second; i++)
			{
				arr[i->second] = value;
			}
			query++;
			break;
		}
	}
}
void kr::TemplatePage::process(HttpClient * client)
{
	Array<Text> variables(m_keys.size());
	// TODO: set variables
	debug();
	size_t n = m_splitContents.size()-1;
	size_t i = 0;
	for (;i<n;i++)
	{
		client->write(m_splitContents[i]);
		client->write(variables[i]);
	}
	client->write(m_splitContents[i]);
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
void MemoryPage::process(HttpClient * client)
{
	client->write(m_contents);
	client->flush();
	client->close();
}

#endif