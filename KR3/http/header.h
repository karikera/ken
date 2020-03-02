#pragma once

#include <KR3/main.h>
#include <KR3/util/uts.h>
#include <KR3/net/socket.h>
#include <KR3/data/map.h>
#include <KR3/data/strstore.h>

#define KR_USER_AGENT "KenLib/1.0"
#define KR_HTTP_SERVER_NAME "KenLibServer"

namespace kr
{
	class HeaderStore;
	class HeaderView;

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
		void set(const HeaderView* data) noexcept;
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
		void set(const HeaderView* data) noexcept;
	};

	class HeaderView
	{
	public:
		HeaderView() noexcept;
		~HeaderView() noexcept;
		void setAll(Text header) noexcept;
		void clear() noexcept;
		Text get(Text name) const noexcept;
		template <typename LAMBDA>
		bool ifGet(Text name, LAMBDA&& call) const noexcept
		{
			return m_map.ifGet(name, forward<LAMBDA>(call));
		}

		Text operator [](Text name) const noexcept;

	protected:
		Map<Text, Text> m_map;
	};
	class HeaderStore
	{
	public:
		HeaderStore() noexcept;
		HeaderStore(HttpStatus status, Text statusName) noexcept;
		~HeaderStore() noexcept;
		Text getStatusLine() noexcept;
		void setStatus(HttpStatus status, Text statusName) noexcept;
		void clear() noexcept;
		void setAll(Text header) noexcept;
		Text get(Text name) const noexcept;
		void set(Text name, Text value) noexcept;
		void setIfNotSetted(Text name, Text value) noexcept;
		void inherit(const HeaderStore* headers) noexcept;

		class Iterator
		{
		public:
			Iterator() noexcept;
			Iterator(HeaderStore* store, Map<Text, size_t>::iterator iter) noexcept;

			Iterator& operator ++() noexcept;
			const Iterator operator ++(int) noexcept;
			bool operator ==(const Iterator& other) const noexcept;
			bool operator !=(const Iterator& other) const noexcept;
			std::pair<Text, Text> operator *() const noexcept;
			Text key() const noexcept;
			Text value() const noexcept;

		private:
			HeaderStore* m_store;
			Map<Text, size_t>::iterator m_iter;
		};

		Iterator begin() noexcept;
		Iterator end() noexcept;

		template <typename LAMBDA>
		bool ifGet(Text name, LAMBDA&& call) const noexcept
		{
			Text value = get(name);
			if (value != nullptr)
			{
				call(value);
				return true;
			}
			else
			{
				return false;
			}
		}

		Text operator [](Text name) const noexcept;

	private:
		AText m_status;
		StringStore<char> m_headerValues;
		Map<Text, size_t> m_headers;
	};

	struct HttpConnectionRequest
	{
		Text contentType;
		Text contentData;
		Text extraHeaders;

		HttpConnectionRequest() noexcept;
	};

}
