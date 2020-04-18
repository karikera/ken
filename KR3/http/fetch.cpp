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

#ifndef __EMSCRIPTEN__
	void setBasic(CURL* curl, curl_slist* headers) noexcept
	{
		if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	}

	AText fetchAsTextImpl(CURL * curl, curl_slist * headers) throws(HttpException)
	{
		AText response;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (CurlWriteCb)[](void *ptr, size_t size, size_t nmemb, void *context)->size_t {
			*((AText*)context) << Text((char*)ptr, size * nmemb);
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
#endif
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
Promise<AText>* HttpRequest::fetchAsText(const char * url) noexcept
{
#ifdef __EMSCRIPTEN__
	m_obj->attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
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
	return threading([curl, headers, postdata = move(m_postdata)]{
		finally{ curl_easy_cleanup(curl); };
		setBasic(curl, headers);
		return fetchAsTextImpl(curl, headers);
	});
#endif
}
#ifndef __EMSCRIPTEN__
AText HttpRequest::fetchAsTextSync(const char * url) throws(HttpException)
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	curl_slist * headers = m_headers;
	m_headers = nullptr;
	setBasic(m_curl, headers);
	AText response = fetchAsTextImpl(m_curl, headers);
	HttpStatus code = _getStatusCode();
	if (code != HttpStatus::OK) throw HttpException(code);
	return response;
}
#endif
#ifndef NO_USE_FILESYSTEM
Promise<void>* HttpRequest::fetchAsFile(const char * url, AText16 filename) noexcept
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	filename.c_str();

	CURL * curl = m_curl;
	curl_slist * headers = m_headers;
	m_curl = nullptr;
	m_headers = nullptr;
	return threading([curl, headers, postdata = move(m_postdata), filename = move(filename)]{
		finally{ curl_easy_cleanup(curl); };
		try
		{
			setBasic(curl, headers);
			return fetchAsFileImpl(curl, File::create(filename.data()), headers);
		}
		catch (...)
		{
			File::remove(filename.data());
			throw;
		}
		});
}
void HttpRequest::fetchAsFileSync(const char * url, pcstr16 filename) throws(HttpException, Error)
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url);

	curl_slist * headers = m_headers;
	m_headers = nullptr;
	setBasic(m_curl, headers);
	fetchAsFileImpl(m_curl, File::create(filename), headers);
	HttpStatus code = _getStatusCode();
	if (code != HttpStatus::OK) throw HttpException(code);
}
#endif
#ifndef __EMSCRIPTEN__
HttpStatus HttpRequest::_getStatusCode() noexcept
{
	long status;
	curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &status);
	return (HttpStatus)status;
}
#endif

Promise<AText>* kr::fetchAsText(Text16 url) noexcept
{
#ifdef NO_USE_FILESYSTEM
	return fetchAsTextFromWeb(TSZ() << toUtf8(url));
#else
	if (url.startsWith(u"http"))
	{
		Text16 next = url.subarr(4);
		if (next.startsWith(u':') || next.startsWith(u"s:"))
		{
			return fetchAsTextFromWeb(TSZ() << toUtf8(url));
		}
	}
	return threading([textsz = AText16::concat(url, nullterm)]{
		return (AText)File::openAsArray<char>(textsz.data());
	});
#endif
}
Promise<AText>* kr::fetchAsText(Text url) noexcept
{
#ifdef NO_USE_FILESYSTEM
	return fetchAsTextFromWeb(TSZ() << url);
#else
	if (url.startsWith("http"))
	{
		Text next = url.subarr(4);
		if (next.startsWith(':') || next.startsWith("s:"))
		{
			return fetchAsTextFromWeb(TSZ() << url);
		}
	}
	return threading([textsz = AText16::concat(utf8ToUtf16(url), nullterm)]{
		return (AText)File::openAsArray<char>(textsz.data());
	});
#endif
}
Promise<AText>* kr::fetchAsTextFromWeb(const char * url) noexcept
{
	HttpRequest req;
	return req.fetchAsText(url);
}
#ifndef NO_USE_FILESYSTEM
Promise<void>* kr::fetchAsFile(Text16 url, AText16 filename) noexcept
{
	return fetchAsFile(TSZ() << toUtf8(url), move(filename));
}
Promise<void>* kr::fetchAsFile(Text url, AText16 filename) noexcept
{
	return fetchAsFileFromSz(TSZ() << url, move(filename));
}
Promise<void>* kr::fetchAsFileFromSz(const char * url, AText16 filename) noexcept
{
	HttpRequest req;
	return req.fetchAsFile(url, move(filename));
}
#endif
