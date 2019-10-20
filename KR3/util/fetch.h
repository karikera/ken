#pragma once

#include <KR3/mt/thread.h>
#include <KRMessage/promise.h>
#include "httpstatus.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#endif


struct curl_slist;
struct curl_httppost;

namespace kr
{
	class HttpRequest
	{
	public:
		HttpRequest() noexcept;
		~HttpRequest() noexcept;

		void setMethod(const char * method) noexcept;
		void setRequestHeader(const char * line) noexcept;
		void setPostFields(AText data) noexcept;
		Promise<AText>* fetchAsText(const char * url) noexcept;
#ifndef __EMSCRIPTEN__
		AText fetchAsTextSync(const char * url) throws(HttpException);
#endif
#ifndef NO_USE_FILESYSTEM
		Promise<void>* fetchAsFile(const char * url, AText16 filename) noexcept;
		void fetchAsFileSync(const char * url, pcstr16 filename) throws(HttpException, Error);
#endif

	private:
#ifdef __EMSCRIPTEN__
		struct FetchObject;
		FetchObject * m_obj;
#else
		HttpStatus _getStatusCode() noexcept;

		void * m_curl;
		curl_slist * m_headers;
		AText m_postdata;
#endif
	};
	Promise<AText>* fetchAsText(Text16 url) noexcept;
	Promise<AText>* fetchAsText(Text url) noexcept;
	Promise<AText>* fetchAsTextFromWeb(const char * url) noexcept;
#ifndef NO_USE_FILESYSTEM
	Promise<void>* fetchAsFile(Text16 url, AText16 filename) noexcept;
	Promise<void>* fetchAsFile(Text url, AText16 filename) noexcept;
	Promise<void>* fetchAsFileFromSz(const char * url, AText16 filename) noexcept;
#endif
}
