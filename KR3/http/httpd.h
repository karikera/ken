#pragma once

#include <KR3/io/selfbufferedstream.h>
#include <KR3/net/socket.h>
#include <KR3/net/mtnet.h>
#include <KR3/mt/thread.h>
#include <KR3/data/map.h>
#include <KR3/http/httpstatus.h>

#include "header.h"

namespace kr
{
	class Page;
	template <typename LAMBDA>
	class LambdaPage;
	class HttpClient;
	class HttpServer;

	class Page
	{
	public:
		Page() noexcept;
		virtual ~Page() noexcept;
		virtual void process(HttpClient * client) = 0;
		template <typename LAMBDA>
		static Page* make(LAMBDA &&lambda) noexcept
		{
			return _new LambdaPage<decay_t<LAMBDA> >(forward<LAMBDA>(lambda));
		}
	private:
	};

	template <typename LAMBDA>
	class LambdaPage:public Page
	{
	public:
		LAMBDA m_lambda;

		LambdaPage(const LAMBDA &lambda) noexcept
			:m_lambda(lambda)
		{
		}

		LambdaPage(LAMBDA&& lambda) noexcept
			:m_lambda(move(lambda))
		{
		}

		void process(HttpClient* client) override
		{
			m_lambda(client);
		}
	};
	
	class TemplatePage:public Page
	{
	public:
		TemplatePage(pcstr16 filename) noexcept;
		~TemplatePage() noexcept override;
		void parseQuery(Array<Text> &arr, Text prefix, Text query) noexcept;
		void process(HttpClient * client) override;

	private:
		AText m_contents;
		Array<Text> m_splitContents;
		std::unordered_multimap<std::string, size_t> m_keys;
	};

	class StaticPage :public Page
	{
	public:
		StaticPage(Text text) noexcept;
		void process(HttpClient * client) override;

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

		void process(HttpClient * client) override;

	private:
		AText m_contents;

	};

	struct HttpFindPage
	{
		Page * page;
		Keep<File> file;
		AText path;
	};

	class UriFormData:public Map<Text, Text, true>
	{
	public:
		UriFormData(Text data) noexcept;
		
	};

	class MultipartFormData
	{
	public:
		struct FormData
		{
			AText header;
		};
		MultipartFormData() noexcept;

		void readHeader(HttpClient* client) throws(ThrowRetry, HttpStatus);
		void read(HttpClient* client, BufferQueue* receive) throws(ThrowRetry, HttpStatus);
		Text get(Text name) noexcept;

		Array<FormData> m_items;
		size_t m_contentSize;
		AText m_boundary;

		AText m_currentHeader;
		enum class State
		{
			SkipBoundary,
			Header,
			Data,
		};
		State m_state;
	};

	class HttpClient :public MTClient
	{
	public:
		class Lock :public MTClient::Lock
		{
			friend HttpClient;
			using Super = MTClient::Lock;
		public:
			~Lock() noexcept;
			void write(Text buffer) noexcept;
			void writes(View<Text> buffers) noexcept;
			void writeHeader(HeaderStore* header = nullptr) noexcept;

		protected:
			Lock(HttpClient* client) noexcept;
		};
		HttpClient(HttpServer* server, Socket* socket) noexcept;
		~HttpClient() noexcept;
		void onRead() throws(...) override;
		void onSendDone() noexcept override;

		void inheritFileHeader(HeaderStore* header) noexcept;

		Lock lock() noexcept;
		void sendText(Text text) noexcept;
		void sendFile(AText path) noexcept;
		void sendFile(AText path, File* file) noexcept;
		void onError(Text funcname, int code) noexcept override;

		Text getMethod() noexcept;
		Text getPath() noexcept;
		Text getQuery() noexcept;
		AText getPostData() throws(ThrowRetry, NotEnoughSpaceException);
		MultipartFormData& getMultipartFormData() throws(ThrowRetry, NotEnoughSpaceException);

		HeaderView* requestHeaders() noexcept;

		HttpServer* const server;

	private:
		void _readHeadLine() throws(ThrowRetry, NotEnoughSpaceException);

		enum class State
		{
			ReadHeader,
			ReadPostData,
			ProcessPage,
			ProcessErrorPage,
			SendFile,
			IgnoreReceive
		};
		AText m_headLine;
		AText m_headerBuffers;
		MultipartFormData m_multipart;
		HeaderView m_requestHeaders;

		State m_state;
		Text m_method;
		Text m_uriRequest;
		Text m_path;
		Text m_query;
		HttpFindPage m_fp;
		bool m_headerParsed : 1;
		bool m_headerSended : 1;
	};


	class HttpServer:public MTServer
	{
	public:

		HttpServer(AText16 htmlRoot) throws(FunctionError);
		~HttpServer() noexcept;

		HeaderStore* defaultHeaders() noexcept;
		void setErrorPage(HttpStatus exception, pcstr16 filename) noexcept;
		void setTemplatePage(Text url, pcstr16 filename) noexcept;
		void setMIMEType(Text type, Text mime) noexcept;
		Text getMIMEType(Text fileName) noexcept;
		void attachPage(Text url, Page * page) noexcept;
		void findPage(Text path, HttpFindPage * fp) noexcept;
		Page * getErrorPage(HttpStatus code) noexcept;

		void onError(Text funcname, int error) noexcept override;
		MTClient* onAccept(Socket* socket) noexcept override;

		template <typename LAMBDA>
		void onPage(Text path, LAMBDA&& lambda) noexcept
		{
			attachPage(path, _new LambdaPage<LAMBDA>(forward<LAMBDA>(lambda)));
		}
		
	private:
		HeaderStore m_headers;
		std::unordered_map<HttpStatus, Page*> m_error;
		AText16 m_htmlRoot;
		Map<Text, Page*> m_map;
		Map<Text, AText> m_mime;
	};

}
