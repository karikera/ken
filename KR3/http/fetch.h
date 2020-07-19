#pragma once

#include <KR3/mt/thread.h>
#include <KR3/msg/promise.h>
#include "httpstatus.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#endif


struct curl_slist;
struct curl_httppost;

namespace kr
{
	struct AtomicProgress
	{
		atomic<uint64_t> uploadNow;
		atomic<uint64_t> uploadTotal;
		atomic<uint64_t> downloadNow;
		atomic<uint64_t> downloadTotal;
		AtomicProgress() noexcept;
	};
	class HttpRequest
	{
	public:
		HttpRequest() noexcept;
		~HttpRequest() noexcept;

		void setMethod(const char * method) noexcept;
		void setRequestHeader(const char * line) noexcept;
		void setPostFields(AText data) noexcept;
		Promise<AText>* fetchAsText(const char * url, AtomicProgress* progress = nullptr) noexcept;
#ifndef __EMSCRIPTEN__
		AText fetchAsTextSync(const char * url, AtomicProgress* progress = nullptr) throws(HttpException);
#endif
#ifndef NO_USE_FILESYSTEM
		Promise<void>* fetchAsFile(const char * url, AText16 filename, AtomicProgress* progress = nullptr) noexcept;
		void fetchAsFileSync(const char * url, pcstr16 filename, AtomicProgress* progress = nullptr) throws(HttpException, Error);
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
		AtomicProgress m_progress;
#endif
	};
	Promise<AText>* fetchAsText(Text16 url, AtomicProgress* progress = nullptr) noexcept;
	Promise<AText>* fetchAsText(Text url, AtomicProgress* progress = nullptr) noexcept;
	Promise<AText>* fetchAsTextFromWeb(const char * url, AtomicProgress* progress = nullptr) noexcept;
#ifndef NO_USE_FILESYSTEM
	Promise<void>* fetchAsFile(Text16 url, AText16 filename, AtomicProgress* progress = nullptr) noexcept;
	Promise<void>* fetchAsFile(Text url, AText16 filename, AtomicProgress* progress = nullptr) noexcept;
	Promise<void>* fetchAsFileFromSz(const char * url, AText16 filename, AtomicProgress* progress = nullptr) noexcept;
#endif
}
