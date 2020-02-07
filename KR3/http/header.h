#pragma once

#include <KR3/main.h>
#include <KR3/util/uts.h>
#include <KR3/net/socket.h>
#include <KR3/data/map.h>

namespace kr
{
	class HttpHeader;
	class AHttpHeader;

	struct CHAHECONTROL
	{
		dword maxage;
	};
	struct KEEPALIVE
	{
		dword timeout;
	};


	struct HttpResponseHeader
	{
		UnixTimeStamp date;
		AText server;
		CHAHECONTROL cacheControl;
		dword expires;
		UnixTimeStamp lastModified;
		AText eTag;
		dword contentLength;
		KEEPALIVE keepAlive;
		AText connection;
		AText contentType;
		AText location;

		HttpResponseHeader() noexcept;
		void set(const HttpHeader & data) noexcept;
	};

	struct HttpRequestHeader
	{
		AText upgrade;
		AText connection;
		AText host;
		AText origin;
		AText wsKey;
		AText wsProtocol;
		uint wsVersion;

		HttpRequestHeader() noexcept;
		void set(const HttpHeader &data) noexcept;
	};

	class HttpHeader
	{
	public:
		HttpHeader() noexcept;
		void set(Text header) noexcept;
		void clear() noexcept;
		Text operator [](Text name) const noexcept;
		template <typename LAMBDA>
		bool ifGet(Text name, LAMBDA&& call) const noexcept
		{
			return m_map.ifGet(name, call);
		}

	private:
		Map<Text, Text, true> m_map;
	};

	class AHttpHeader : public HttpHeader
	{
	public:
		AHttpHeader() noexcept;
		~AHttpHeader() noexcept;
		void set(AText header) noexcept;

	private:
		AText m_buffer;
	};

	struct HttpConnectionRequest
	{
		Text contentType;
		Text contentData;
		Text extraHeaders;

		HttpConnectionRequest() noexcept;
	};

}
