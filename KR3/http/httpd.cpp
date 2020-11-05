#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else

#include "httpd.h"

#include <KR3/fs/file.h>
#include <KR3/initializer.h>
#include <KR3/mt/thread.h>
#include <KR3/msg/pool.h>
#include <KR3/win/windows.h>
#include <KRTool/mimetypes/gen/mime.h>

using namespace kr;

inline AText readWithHeader(Text header, pcstr16 path)
{
	Must<File> file = File::open(path);
	dword filesize = file->size32();
	size_t headersize = header.size();
	AText contents;
	contents.resize(headersize + filesize);
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
	
	auto* headers = client->requestHeaders();

	Text contentLength = headers->get("Content-Length");
	if (contentLength == nullptr) throw HttpStatus::MethodNotAllowed;
	m_contentSize = contentLength.to_uintp();

	Text contentType = headers->get("Content-Type");
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
	HashTester<void> boundary(m_boundary, HashTester<void>::NoReset);
	HashTester<void> lineend("\r\n"_tx, HashTester<void>::NoReset);
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
			//BufferQueue::ReadResult res = receive->readto(boundary);
			//size_t junksize = res.totalSize + boundary.size();
			//m_contentSize -= junksize;
			//receive->skip(junksize);
			//if (!res.ended) throw ThrowRetry();
			//m_state = State::Header;
			break;
		}
	}
}
Text MultipartFormData::get(Text name) noexcept
{
	return "";
}

HttpClient::Lock::Lock(HttpClient* client) noexcept
	:Super(client)
{
}
HttpClient::Lock::~Lock() noexcept
{
}
void HttpClient::Lock::write(Text buffer) noexcept
{
	Super::write(buffer);
}
void HttpClient::Lock::writes(View<Text> buffers) noexcept
{
	for (Text buf : buffers)
	{
		write(buf);
	}
}
void HttpClient::Lock::writeHeader(HeaderStore* header) noexcept
{
	HttpClient* client = static_cast<HttpClient*>(m_client);
	if (client->m_headerSended) return;
	client->m_headerSended = true;

	Manual<HeaderStore> tempstore;

	if (header == nullptr)
	{
		tempstore.create();
		header = tempstore;
	}
	UnixTimeStamp now = UnixTimeStamp::now();
	header->setIfNotSetted("Date", TSZ() << now.getInfo());
	header->inherit(client->server->defaultHeaders());

	write(header->getStatusLine());
	for (auto pair : *header)
	{
		write(pair.first);
		write(": ");
		write(pair.second);
		write("\r\n");
	}
	write("\r\n");

	if (header == tempstore.address())
	{
		tempstore.remove();
	}
}

HttpClient::HttpClient(HttpServer* server, Socket* socket) noexcept
	:MTClient(socket), server(server),
	m_headerParsed(false),
	m_headerSended(false)
{
	m_state = State::ReadHeader;

	CsLock __lock = doutLock;
	clientCount++;
	dout << "HTTP Client Count: " << clientCount << endl;
}
HttpClient::~HttpClient() noexcept
{
	m_requestHeaders.clear();

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
				if (*url != '/') throw HttpStatus::BadRequest;
				url++;
				const char* query = url.find('?');
				if (query != nullptr)
				{
					m_query = url.subarr(query);
					m_path = url.cut(query);
					query++;
				}
				else
				{
					m_query = Text(url.end(), url.end());
					m_path = url;
				}

				server->findPage(m_path, &m_fp);
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
				m_fp.page = server->getErrorPage(HttpStatus::BadRequest);
				m_state = State::ProcessErrorPage;
			}
			catch (HttpStatus status)
			{
				m_fp.page = server->getErrorPage(status);
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
				m_fp.page = server->getErrorPage(HttpStatus::BadRequest);
				m_state = State::ProcessErrorPage;
			}
			catch (HttpStatus code)
			{
				m_fp.page = server->getErrorPage(code);
				m_state = State::ProcessErrorPage;
			}
			catch (...)
			{
				m_fp.page = server->getErrorPage(HttpStatus::InternalServerError);
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
				lock().closeClient();
			}
			return;
		case State::SendFile:
		{
			HeaderStore header;
			inheritFileHeader(&header);
			lock().writeHeader(&header);

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
			TmpArray<char> buffer;
			buffer.resize(8192);
			buffer.resize(m_fp.file->$read(buffer.begin(), buffer.size()));

			if (Lock _lock = lock())
			{
				_lock.write(buffer);
				_lock.flush();
			}
		}
		catch (EofException&)
		{
			m_fp.file = nullptr;
			lock().closeClient();
		}
	}
}
void HttpClient::inheritFileHeader(HeaderStore* header) noexcept
{
	header->setIfNotSetted("Content-Type", server->getMIMEType(m_fp.path));
	header->setIfNotSetted("Content-Length", TSZ() << m_fp.file->size());
}
HttpClient::Lock HttpClient::lock() noexcept
{
	return _writeLock() ? this : nullptr;
}
void HttpClient::sendText(Text text) noexcept
{
	if (Lock _lock = lock())
	{
		_lock.writeHeader();
		_lock.write(text);
		_lock.flush();
		_lock.closeClient();
	}
}
void HttpClient::sendFile(AText path) noexcept
{
	sendFile(move(path), File::open(TSZ16() << (Utf8ToUtf16)path));
}
void HttpClient::sendFile(AText path, File* file) noexcept
{
	m_fp.page = nullptr;
	m_fp.path = path;
	m_fp.file = move(file);

	threadingVoid([this] {
		onSendDone();
		});
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
	Text value = requestHeaders()->get("Content-Length");
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

HeaderView* HttpClient::requestHeaders() noexcept
{
	if (!m_headerParsed)
	{
		HashTester<void> needle = (Buffer)"\r\n\r\n"_tx;
		m_receive.readto(needle).readAllTo((ABuffer*)&m_headerBuffers, HEADER_LENGTH_LIMIT);
		m_receive.skip(needle.size());
		m_requestHeaders.setAll(m_headerBuffers);
		m_headerParsed = true;
	}
	return &m_requestHeaders;
}

void HttpClient::_readHeadLine() throws(ThrowRetry, NotEnoughSpaceException)
{
	HashTester<void> needle = (Buffer)"\r\n"_tx;
	m_receive.readto(needle).readAllTo((ABuffer*)&m_headLine, HEADER_LENGTH_LIMIT);
	m_receive.skip(needle.size());
}

HttpServer::HttpServer(AText16 htmlRoot) throws(FunctionError)
	:m_htmlRoot(move(htmlRoot))
{
	if (m_htmlRoot.endsWith_y(u"\\/")) m_htmlRoot.pop();

	gen::initMimeMap(&m_mime);

	m_headers.set("Server", KR_HTTP_SERVER_NAME);
	// m_headers.set("Transfer-Encoding", "identity");
	m_headers.set("MIME-Version", "1.0");
	m_headers.set("Content-Type", "text/html; charset=utf-8");
	m_headers.set("Connection", "close");
	// Allow: GET, HEAD, PUT // 405(Method Not Allowed)
	// WWW-Authenticate: Newauth realm="apps", type=1, title = "Login to \"apps\"", Basic realm = "simple" // 401(Unauthorized)

	m_error[HttpStatus::NotModified] =  _new StaticPage("HTTP/1.0 304 Not Modified\r\n\r\n");
	m_error[HttpStatus::NotImplemented] =  _new StaticPage("HTTP/1.0 501 Not Implemented\r\nContent-Type: text/plain\r\n\r\nNot implemented");
	m_error[HttpStatus::NotFound] = _new StaticPage("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found");
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

HeaderStore* HttpServer::defaultHeaders() noexcept
{
	return &m_headers;
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
	Text ext = path.extname(fileName);
	if (ext.empty())
	{
		return "application/octet-stream";
	}
	else
	{
		auto iter2 = m_mime.find(ext+1);
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
		AText nurl;
		nurl.reserve(path.size() + 32);
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
	HttpClient::Lock _lock = client->lock();
	_lock.write(m_contents);
	_lock.flush();
	_lock.closeClient();
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
	Array<Text> variables;
	variables.resize(m_keys.size());
	// TODO: set variables
	debug();
	HttpClient::Lock _lock = client->lock();
	size_t n = m_splitContents.size()-1;
	size_t i = 0;
	for (;i<n;i++)
	{
		_lock.write(m_splitContents[i]);
		_lock.write(variables[i]);
	}
	_lock.write(m_splitContents[i]);
	_lock.flush();
	_lock.closeClient();
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
	HttpClient::Lock _lock = client->lock();
	_lock.write(m_contents);
	_lock.flush();
	_lock.closeClient();
}

#endif