#include "stdafx.h"
#include "fetch.h"

#ifndef __EMSCRIPTEN__

#include "header.h"
#define CURL_STATICLIB

#include "curl/curl.h"
#endif

#ifndef NO_USE_FILESYSTEM

#include <KR3/fs/file.h>

#endif

#include <KR3/io/bufferedstream.h>
#include <KR3/msg/pool.h>

#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Normaliz.lib")

using namespace kr;

namespace
{
	bool initCurl = false;

	using CurlWriteCb = size_t(*)(void *contents, size_t size, size_t nmemb, void *userp);
	using CurlXferInfoCb = int(*)(void* contents, 
		curl_off_t dltotal, curl_off_t dlnow,
		curl_off_t ultotal, curl_off_t ulnow);

	void urlToFilename(AText16* out, Text16 url) noexcept {
		_assert(url.startsWith(u"http"));
		Text16 next = url.subarr(4);
		if (next.startsWith(u':')) {
			next++;
		}
		else {
			_assert(next.startsWith(u"s:"));
			next += 2;
		}

		out->prepare(url.size() * 3 / 2);
		while (!next.empty()) {
			char16_t chr = *next++;
			_assert(chr != '\0');
			switch (chr) {
			case '\\':
			case '/':
				*out << '#';
				break;
			case '*':
				*out << '^';
				break;
			default:
				*out << chr;
				break;
			}
		}
	}

#ifndef __EMSCRIPTEN__
	void setBasic(CURL* curl, curl_slist* headers) noexcept
	{
		if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_WRITE_SIZE);
	}

	void setProgressImpl(CURL* curl, AtomicProgress* progress) noexcept
	{
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*)progress);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, (CurlXferInfoCb)[](void* context,
			curl_off_t dltotal, curl_off_t dlnow,
			curl_off_t ultotal, curl_off_t ulnow)->int{
				AtomicProgress* prog = ((AtomicProgress*)context);
				prog->uploadNow = ulnow;
				prog->uploadTotal = ultotal;
				prog->downloadNow = dlnow;
				prog->downloadTotal = dltotal;
				return 0;
			});
	}
	AText fetchAsTextImpl(CURL * curl, curl_slist * headers) throws(HttpException)
	{
		AText response;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (CurlWriteCb)[](void *ptr, 
			size_t size, size_t nmemb, void *context)->size_t {
			(*(AText*)context) << Text((char*)ptr, size * nmemb);
			return nmemb;
		});
		CURLcode res = curl_easy_perform(curl);		
		curl_slist_free_all(headers);
		if (res != CURLE_OK)
		{
#ifndef NDEBUG
			dout << curl_easy_strerror(res) << endl;
#endif
			throw HttpException{ HttpStatus::Undefined };
		}
		else
		{
			return response;
		}
	}
#endif

#ifndef NO_USE_FILESYSTEM
	void fetchAsFileImpl(CURL * curl, File * file, curl_slist * headers) throws(HttpException)
	{
		io::FOStream<char> stream = file;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&stream);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (CurlWriteCb)[](void *ptr, size_t size, size_t nmemb, void *context)->size_t {
			*((io::FOStream<char>*)context) << Text((char*)ptr, size * nmemb);
			return nmemb;
		});
		CURLcode res = curl_easy_perform(curl);
		curl_slist_free_all(headers);
		if (res != CURLE_OK)
		{
#ifndef NDEBUG
			dout << curl_easy_strerror(res) << endl;
#endif
			throw HttpException{ HttpStatus::Undefined };
		}
	}
	void fetchAsFileImpl(CURL* curl, Text16 filename_sz, curl_slist* headers, AtomicProgress* progress) throws(HttpException, Error) {
		if (File::exists(filename_sz.data())) return;

		BText16<MAX_PATH> tempfile;
		tempfile << filename_sz << u".#temp" << nullterm;

		try
		{
			setBasic(curl, headers);
			if (progress) setProgressImpl(curl, progress);
			fetchAsFileImpl(curl, File::create(tempfile.data()), headers);
		}
		catch (...)
		{
			File::remove(tempfile.data());
			throw;
		}
		if (!File::move(filename_sz.data(), tempfile.data())) {
			File::remove(tempfile.data());
		}
	}
#endif
}

AtomicProgress::AtomicProgress() noexcept
	:uploadNow(0), uploadTotal(0), downloadNow(0), downloadTotal(0)
{
}

#ifdef __EMSCRIPTEN__
struct HttpRequest::FetchObject :public DeferredPromise<AText>
{
	FetchObject() noexcept;
	~FetchObject() noexcept override;
	emscripten_fetch_attr_t attr;
	AText postdata;
	AText requestHeaders;
	Array<size_t> requestHeaderIndexes;
};
HttpRequest::FetchObject::FetchObject() noexcept
{
	emscripten_fetch_attr_init(&attr);
	attr.userData = this;
	memcpy(attr.requestMethod, "GET", 4);
}
HttpRequest::FetchObject::~FetchObject() noexcept
{
}
#endif

HttpRequest::HttpRequest() noexcept
{
#ifdef __EMSCRIPTEN__
	m_obj = _new FetchObject;
#else
	if (!initCurl)
	{
		initCurl = true;
		curl_global_init(CURL_GLOBAL_ALL);
	}

	m_headers = curl_slist_append(nullptr, "User-Agent: " KR_USER_AGENT);
	m_curl = curl_easy_init();
	if (!m_curl) notEnoughMemory();
#endif
}
HttpRequest::~HttpRequest() noexcept
{
#ifdef __EMSCRIPTEN__
	delete m_obj;
#else
	if (m_headers) curl_slist_free_all(m_headers);
	if (m_curl) curl_easy_cleanup(m_curl);
#endif
}
void HttpRequest::setMethod(const char * method) noexcept
{
#ifdef __EMSCRIPTEN__
	_assert(sizeof(m_obj->attr.requestMethod) > strlen(method));
	strcpy(m_obj->attr.requestMethod, method);
#else
	curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, method);
#endif
}
void HttpRequest::setRequestHeader(const char * line) noexcept
{
#ifdef __EMSCRIPTEN__
	Text txline = (Text)line;
	const char* txnext = txline.find(':');
	if (txnext == nullptr) return;
	txnext++;
	if (txnext == txline.end()) return;
	if (*txnext == ' ') txnext++;
	size_t idx = m_obj->requestHeaders.size();
	m_obj->requestHeaderIndexes.push(idx);
	m_obj->requestHeaderIndexes.push(idx + (txnext - txline.data()));
	m_obj->requestHeaders << txline;
#else
	m_headers = curl_slist_append(m_headers, line);
#endif
}
void HttpRequest::setPostFields(AText data) noexcept
{
#ifdef __EMSCRIPTEN__
	m_obj->postdata = move(data);
	memcpy(m_obj->attr.requestMethod, "POST", 5);
	m_obj->attr.requestData = m_obj->postdata.data();
	m_obj->attr.requestDataSize = m_obj->postdata.size();
#else
	m_postdata = move(data);
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_postdata.data());
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, intact<long>(m_postdata.size()));
#endif
}
Promise<AText>* HttpRequest::fetchAsText(const char * url, AtomicProgress* progress) noexcept
{
#ifdef __EMSCRIPTEN__
	m_obj->attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	if (progress)
	{
		m_obj->attr.onprogress = [] {
		}
	};
	m_obj->attr.onsuccess = [](emscripten_fetch_t *fetch) {
		auto * obj = (FetchObject*)fetch->userData;
		obj->_resolve(AText(fetch->data, fetch->numBytes));
		emscripten_fetch_close(fetch);
		obj->postdata = nullptr;
		obj->requestHeaders = nullptr;
		obj->requestHeaderIndexes = nullptr;
		PromisePump::getInstance()->process();
	};
	m_obj->attr.onerror = [](emscripten_fetch_t *fetch) {
		auto * obj = (FetchObject*)fetch->userData;
		obj->_reject(HttpException((HttpStatus)fetch->status));
		emscripten_fetch_close(fetch);
		obj->postdata = nullptr;
		obj->requestHeaders = nullptr;
		obj->requestHeaderIndexes = nullptr;
		PromisePump::getInstance()->process();
	};
	emscripten_fetch(&m_obj->attr, url);
	FetchObject * obj = m_obj;
	m_obj = nullptr;
	return obj;
#else
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	CURL * curl = m_curl;
	curl_slist * headers = m_headers;
	m_curl = nullptr;
	m_headers = nullptr;
	return threading([curl, headers, postdata = move(m_postdata), progress]{
		finally{ curl_easy_cleanup(curl); };
		setBasic(curl, headers);
		if (progress) setProgressImpl(curl, progress);
		return fetchAsTextImpl(curl, headers);
	});
#endif
}
Promise<AText>* HttpRequest::fetchAsTextCached(const char* url, AtomicProgress* progress) noexcept {
#ifdef __EMSCRIPTEN__
	return fetchAsText(url, progress);
#else
	curl_easy_setopt(m_curl, CURLOPT_URL, url);

	CURL* curl = m_curl;
	curl_slist* headers = m_headers;
	m_curl = nullptr;
	m_headers = nullptr;

	AText16 filename16 = (Utf8ToUtf16)(Text)url;
	AText16 target = u"webcache\\";
	urlToFilename(&target, filename16);

	return threading([curl, headers, postdata = move(m_postdata), target = move(target), progress]() mutable{
		finally { curl_easy_cleanup(curl); };
		File::createDirectory(u"webcache");
		target.c_str();
		fetchAsFileImpl(curl, target, headers, progress);
		return (AText)File::openAsArray<char>(target.data());
	});
#endif
}
#ifndef __EMSCRIPTEN__
AText HttpRequest::fetchAsTextSync(const char * url, AtomicProgress* progress) throws(HttpException) {
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	curl_slist * headers = m_headers;
	m_headers = nullptr;
	setBasic(m_curl, headers);
	if (progress) setProgressImpl(m_curl, progress);
	AText response = fetchAsTextImpl(m_curl, headers);
	HttpStatus code = _getStatusCode();
	if (code != HttpStatus::OK) throw HttpException(code);
	return response;
}
AText HttpRequest::fetchAsTextSyncCached(const char* url, AtomicProgress* progress) throws(HttpException) {
	AText16 filename16 = (Utf8ToUtf16)(Text)url;
	AText16 target = u"webcache\\";
	urlToFilename(&target, filename16);
	target.c_str();

	File::createDirectory(u"webcache");
	fetchAsFileSync(url, target.data(), progress);
	return File::openAsArray<char>(target.data());
}
#endif
#ifndef NO_USE_FILESYSTEM
Promise<void>* HttpRequest::fetchAsFile(const char * url, AText16 filename, AtomicProgress* progress) noexcept
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url);

	CURL * curl = m_curl;
	curl_slist * headers = m_headers;
	m_curl = nullptr;
	m_headers = nullptr;
	
	return threading([curl, headers, postdata = move(m_postdata), filename = move(filename), progress]() mutable{
		finally{ curl_easy_cleanup(curl); };
		filename.c_str();
		fetchAsFileImpl(curl, filename, headers, progress);
		});
}
void HttpRequest::fetchAsFileSync(const char * url, pcstr16 filename, AtomicProgress* progress) throws(HttpException, Error) {
	curl_easy_setopt(m_curl, CURLOPT_URL, url);

	curl_slist * headers = m_headers;
	m_headers = nullptr;
	fetchAsFileImpl(m_curl, (Text16)filename, headers, progress);
	HttpStatus code = _getStatusCode();
	if (code != HttpStatus::OK) throw HttpException(code);
}
#endif
#ifndef __EMSCRIPTEN__
HttpStatus HttpRequest::_getStatusCode() noexcept {
	long status;
	curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &status);
	return (HttpStatus)status;
}
#endif

Promise<AText>* kr::fetch::text(Text16 url, AtomicProgress* progress) noexcept {
#ifdef NO_USE_FILESYSTEM
	return fetchAsTextFromWeb(TSZ() << toUtf8(url));
#else
	if (url.startsWith(u"http")) {
		Text16 next = url.subarr(4);
		if (next.startsWith(u':') || next.startsWith(u"s:")) {
			return fetch::web::text(TSZ() << toUtf8(url), progress);
		}
	}
	return threading([textsz = AText16::concat(url, nullterm)]{
		return (AText)File::openAsArray<char>(textsz.data());
	});
#endif
}
Promise<AText>* kr::fetch::text(Text url, AtomicProgress* progress) noexcept {
#ifdef NO_USE_FILESYSTEM
	return fetchAsTextFromWeb(TSZ() << url);
#else
	if (url.startsWith("http")) {
		Text next = url.subarr(4);
		if (next.startsWith(':') || next.startsWith("s:")) {
			return fetch::web::text(TSZ() << url, progress);
		}
	}
	return threading([textsz = AText16::concat(utf8ToUtf16(url), nullterm)]{
		return (AText)File::openAsArray<char>(textsz.data());
		});
#endif
}
AText kr::fetch::sync::text(Text16 url, AtomicProgress* progress) noexcept {
#ifdef NO_USE_FILESYSTEM
	return fetchAsTextFromWeb(TSZ() << toUtf8(url));
#else
	if (url.startsWith(u"http")) {
		Text16 next = url.subarr(4);
		if (next.startsWith(u':') || next.startsWith(u"s:")) {
			return fetch::web::sync::text(TSZ() << toUtf8(url), progress);
		}
	}
	return (AText)File::openAsArray<char>(TSZ16() << url);
#endif
}
Promise<AText>* kr::fetch::web::text(const char * url, AtomicProgress* progress) noexcept {
	HttpRequest req;
	return req.fetchAsText(url, progress);
}
AText kr::fetch::web::sync::text(const char* url, AtomicProgress* progress) noexcept {
	HttpRequest req;
	return req.fetchAsTextSync(url, progress);
}
Promise<AText>* kr::fetch::web::cached::text(const char* url, AtomicProgress* progress) noexcept {
	HttpRequest req;
	return req.fetchAsTextCached(url, progress);
}
AText kr::fetch::web::cached::sync::text(const char* url, AtomicProgress* progress) noexcept {
	HttpRequest req;
	return req.fetchAsTextSyncCached(url, progress);
}
#ifndef NO_USE_FILESYSTEM
Promise<void>* kr::fetch::file(Text16 url, AText16 filename, AtomicProgress* progress) noexcept
{
	return fetch::file_sz(TSZ() << toUtf8(url), move(filename), progress);
}
Promise<void>* kr::fetch::file(Text url, AText16 filename, AtomicProgress* progress) noexcept
{
	return fetch::file_sz(TSZ() << url, move(filename), progress);
}
Promise<void>* kr::fetch::file_sz(const char * url, AText16 filename, AtomicProgress* progress) noexcept
{
	HttpRequest req;
	return req.fetchAsFile(url, move(filename), progress);
}
#endif

Promise<AText>* kr::fetch::cached::text(Text16 url, AtomicProgress* progress) noexcept {
#ifdef NO_USE_FILESYSTEM
	return fetch::web::text(TSZ() << toUtf8(url));
#else
	if (url.startsWith(u"http")) {
		Text16 next = url.subarr(4);
		if (next.startsWith(u':') || next.startsWith(u"s:")) {
			return fetch::web::cached::text(TSZ() << toUtf8(url), progress);
		}
	}
	return threading([textsz = AText16::concat(url, nullterm)]{
		return (AText)File::openAsArray<char>(textsz.data());
		});
#endif
}
Promise<AText>* kr::fetch::cached::text(Text url, AtomicProgress* progress) noexcept {
#ifdef NO_USE_FILESYSTEM
	return fetch::web::text(TSZ() << url);
#else
	if (url.startsWith("http")) {
		Text next = url.subarr(4);
		if (next.startsWith(':') || next.startsWith("s:")) {
			return fetch::web::cached::text(TSZ() << url, progress);
		}
	}
	return threading([textsz = AText16::concat(utf8ToUtf16(url), nullterm)]{
		return (AText)File::openAsArray<char>(textsz.data());
		});
#endif
}
AText kr::fetch::cached::sync::text(Text16 url, AtomicProgress* progress) noexcept {
#ifdef NO_USE_FILESYSTEM
	return fetchAsTextFromWeb(TSZ() << toUtf8(url));
#else
	if (url.startsWith(u"http")) {
		Text16 next = url.subarr(4);
		if (next.startsWith(u':') || next.startsWith(u"s:")) {
			return fetch::web::cached::sync::text(TSZ() << toUtf8(url), progress);
		}
	}
	return (AText)File::openAsArray<char>(TSZ16() << url);
#endif
}
