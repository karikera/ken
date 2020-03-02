#pragma once

#include <KR3/main.h>
#include <KR3/mt/thread.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/net/socket.h>

#include "wsevent.h"
#include "client.h"
#include "server.h"

namespace kr
{
	struct Operation;
	class MTClient;
	
	class MTClient:public AtomicReferencable<MTClient>
	{
	public:
		class Lock
		{
			friend MTClient;
		public:
			~Lock() noexcept;

			void write(Buffer buffer) noexcept;
			void writes(View<Buffer> buffers) noexcept;
			void flush() noexcept;
			void closeClient() noexcept;

			operator bool() noexcept;
			bool operator !() noexcept;
			bool operator ==(nullptr_t) noexcept;
			bool operator !=(nullptr_t) noexcept;

		protected:
			Lock(MTClient* client) noexcept;

			MTClient* const m_client;
		};
		MTClient() noexcept;
		MTClient(Socket * socket) noexcept;
		MTClient(MTClient* client) noexcept;
		virtual ~MTClient() noexcept;

		void reset(Socket * socket) noexcept;
		Socket * getSocket() noexcept;
		void connect(Ipv4Address v4addr, int port) throws(SocketException);
		void connect(pcstr16 host, int port) throws(SocketException);
		void requestReceive() noexcept;
		Lock lock() noexcept;
		bool isClosed() noexcept;

		void readSetBuffer(BufferQueue receiver) noexcept;
		void readCommit(size_t size) noexcept;
		void writeCommit(size_t size) noexcept;
		void callOnRead() noexcept;

		virtual void onConnect() noexcept;
		virtual void onRead() = 0;
		virtual void onError(Text funcname, int code) noexcept = 0;
		virtual void onSendDone() noexcept;
		virtual void onClose() noexcept;

		template <typename T, typename ... ARGS>
		T* switchClient(ARGS && ... args) noexcept;
		
	protected:
		bool _writeLock() noexcept;
		BufferQueue m_receive;

	private:
		void _destroy() noexcept;
		void _callOnRead() noexcept;
		void _write_wl(Buffer buffer) noexcept;
		void _flush_wl() noexcept;
		void _close_wl() noexcept;

		Socket * m_socket;
		CriticalSection m_cs;
		MTClient * m_switchClient;
		BufferQueue m_writeQueue;
		Operation * m_flushOperation;
		Operation * m_receiveOperation;
		enum class RState :byte
		{
			Processing,
			Receiving,
			Closed,
		};
		enum class SState :byte
		{
			Idle,
			Flushing,
			Processing,
		};
		RState m_receiving;
		SState m_flushing;
		bool m_starting;
		bool m_closing;
		
	};

	class MTServer
	{
	public:
		MTServer() throws(FunctionError);
		~MTServer() noexcept;
		void open(int port) throws(SocketException);
		static void init(uint threadCount) noexcept;
		static void init() noexcept;
		static bool isInited() noexcept;

		virtual MTClient* onAccept(Socket * socket) noexcept = 0;
		virtual void onError(Text message, int errcode) noexcept = 0;

		static int CT_STDCALL iocpWorker(void*) noexcept;
				
	private:
		struct AcceptOperation;
		AText m_htmlPath;
		Socket * m_serverSocket;
		Array<AcceptOperation> m_acceptOperations;
	};
}

template <typename T, typename ... ARGS>
T* kr::MTClient::switchClient(ARGS && ... args) noexcept
{
	T * newclient = _new T(move(args) ... , m_socket);
	_switchClient(newclient);
	return newclient;
}
