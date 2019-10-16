#pragma once

#include <KR3/io/selfbufferedstream.h>
#include <KR3/net/socket.h>
#include <KRMessage/net/mtnet.h>
#include <KR3/mt/thread.h>
#include <KR3/data/map.h>
#include <KR3/util/httpstatus.h>

#include "common/header.h"

namespace kr
{
	class Page;
	class HttpClient;
	class HttpServer;

	class Page
	{
	public:
		Page() noexcept;
		virtual ~Page() noexcept;
		virtual void process(HttpClient * client,Text query, BufferQueue * stream) = 0;
	private:
	};
	
	class TemplatePage:public Page
	{
	public:
		TemplatePage(pcstr16 filename) noexcept;
		~TemplatePage() noexcept override;
		void parseQuery(Array<Text> &arr, Text prefix, Text query) noexcept;
		void process(HttpClient * client, Text query, BufferQueue * stream) override;

	private:
		AText m_contents;
		Array<Text> m_splitContents;
		std::unordered_multimap<std::string, size_t> m_keys;
	};

	class StaticPage :public Page
	{
	public:
		StaticPage(Text text) noexcept;
		void process(HttpClient * client, Text query, BufferQueue * stream) override;

	private:
		Text m_contents;
	};

	class MemoryPage :public Page
	{
	public:
		MemoryPage() noexcept;
		MemoryPage(pcstr16 path) throws(Error, TooBigException);
		MemoryPage(Text header, pcstr16 path) throws(Error);
		MemoryPage(const MemoryPage& _copy) noexcept;
		MemoryPage(MemoryPage&& _move) noexcept;
		~MemoryPage() noexcept;

		MemoryPage & operator =(const MemoryPage& _copy) noexcept;
		MemoryPage & operator =(MemoryPage&& _move) noexcept;

		void process(HttpClient * client, Text query, BufferQueue * stream) override;

	private:
		AText m_contents;

	};

	struct HttpFindPage
	{
		Page * page;
		Keep<File> file;
		AText path;
	};

	class HttpClient:public MTClient
	{
	public:
		HttpClient(HttpServer * server, Socket * socket) noexcept;
		~HttpClient() noexcept;
		void onRead() throws(...) override;
		void onSendDone() noexcept override;
		void onError(Text funcname, int code) noexcept override;

		void readPostVariable(Text * stream);

		void parseQueryTo(ReferenceMap<Text, Text> * target, Text data) noexcept;
		Text getHeader(Text name) noexcept;
		Text get(Text name) noexcept;
		Text post(Text name) noexcept;
		HttpHeader& getHeader() noexcept;
		ReferenceMap<Text, Text>& getMap() noexcept;
		ReferenceMap<Text, Text>& postMap() noexcept;

	private:
		enum State
		{
			ReadHeader,
			ProcessPage,
			ProcessErrorPage,
			SendFile,
			IgnoreReceive
		};
		HttpServer * m_server;
		HttpHeader m_header;
		AText m_headerData;
		State m_state;
		Text m_method;
		Text m_uriRequest;
		Text m_path;
		Text m_query;
		HttpFindPage m_fp;
		ReferenceMap<Text, Text> m_get;
		ReferenceMap<Text, Text> m_post;
		bool m_postParsed : 1;
		bool m_getParsed : 1;
	};


	class HttpServer:public MTServer
	{
	public:

		HttpServer() noexcept;
		~HttpServer() noexcept;

		void setDefaultHeader(pcstr16 filename) noexcept;
		Text getDefaultHeader() noexcept;
		void setErrorPage(HttpStatus exception, pcstr16 filename) noexcept;
		void setTemplatePage(Text url, pcstr16 filename) noexcept;
		void setMIMEType(Text type, Text mime) noexcept;
		Text getMIMEType(Text fileName) noexcept;
		void attachPage(Text url, Page * page) noexcept;
		void findPage(Text path, HttpFindPage * fp) noexcept;
		Page * getErrorPage(HttpStatus code) noexcept;

		void onError(Text funcname, int error) noexcept override;
		MTClient* onAccept(Socket * socket) noexcept override;

	private:
		AText m_headers;
		std::unordered_map<HttpStatus, Page*> m_error;
		Map<Text, Page*> m_map;
		Map<Text, AText> m_mime;
	};

}
