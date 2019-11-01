#pragma once

#include <KR3/main.h>
#include <KR3/net/socket.h>
#include <KR3/net/ipaddr.h>
#include <KR3/wl/eventhandle.h>
#include <KR3/msg/promise.h>

namespace kr
{
	struct FNetworkEvent
	{
		union
		{
			struct
			{
				dword read : 1;
				dword write : 1;
				dword OOB : 1;
				dword accept : 1;
				dword connect : 1;
				dword close : 1;
				dword QOS : 1;
				dword groupQOS : 1;
				dword routingInterfaceChange : 1;
				dword addressListChange : 1;
			};
			dword bits;
		};

		FNetworkEvent() noexcept;
		static FNetworkEvent getDefaultClient() noexcept;
		static FNetworkEvent getDefaultServer() noexcept;
		FNetworkEvent(bool _write, bool _read, bool _connect, bool _close, bool _accept) noexcept;
	};

	struct NetworkState: FNetworkEvent
	{
		union
		{
			struct
			{
				int errRead;
				int errWrite;
				int errOOB;
				int errAccept;
				int errConnect;
				int errClose;
				int errQOS;
				int errGroupQOS;
				int errRoutingInterfaceChange;
				int errAddressListChange;
			};

			struct
			{
				int errors[10];
			};
		};

		static constexpr int maxEventCount = 10;
	};

	class SocketEventHandle : public EventHandle
	{
	public:
		static SocketEventHandle* create() noexcept;
		void operator delete(ptr p) noexcept;
		int select(Socket* sock, FNetworkEvent lNetworkEvents) noexcept;
		int deselect(Socket * sock) noexcept;
		NetworkState getState(Socket* sock) noexcept;
		Promise<void> * waitState(Socket* sock, int fdbit) noexcept;
		Promise<NetworkState> * waitStates(Socket* sock, int fdmask) noexcept;
		void set() noexcept;
		void reset() noexcept;
	};

	namespace _pri_
	{
		class SocketEventListImpl:public EventListImpl
		{
		public:
			SocketEventListImpl() noexcept;
			SocketEventHandle * pickOut(size_t idx) noexcept;
			SocketEventHandle * get(size_t idx) noexcept;

		protected:
			SocketEventHandle ** _ptr() noexcept;
		};
	}

	template <size_t SIZE> class SocketEventList :public _pri_::SocketEventListImpl
	{
	public:
		inline void push(SocketEventHandle * ev) noexcept
		{
			_assert(m_size < SIZE);
			m_events[m_size++] = ev;
		}
		inline SocketEventHandle * create() noexcept
		{
			SocketEventHandle * ev = SocketEventHandle::create();
			push(ev);
			return ev;
		}
		inline Socket * openServer(word port) noexcept
		{
			Socket * sock = Socket::create();
			sock->open(port);
			SocketEventHandle * ev = SocketEventHandle::create();
			ev->select(sock, FNetworkEvent::getDefaultServer());
			push(ev);
			return sock;
		}
		inline size_t remaining() noexcept
		{
			return SIZE - m_size;
		}
		inline void resize(size_t sz) noexcept
		{
			_assert(sz <= SIZE);
			m_size = sz;
		}

	private:
		SocketEventHandle * m_events[SIZE];
	};


	class SocketEvent
	{
	public:
		SocketEvent(SocketEventHandle * handle) noexcept;
		explicit SocketEvent() noexcept;
		SocketEvent(SocketEvent && move) noexcept;
		~SocketEvent() noexcept;
		SocketEvent(const SocketEvent&) = delete;
		SocketEvent& operator = (const SocketEvent&) = delete;
		SocketEventHandle * operator ->() noexcept;
		SocketEventHandle * operator &() noexcept;

		SocketEvent & operator = (SocketEvent && move) noexcept;

	protected:
		SocketEventHandle * m_event;
	};


	class EventedSocket
	{
	public:
		EventedSocket() noexcept;
		~EventedSocket() noexcept;

		void open(word port) throws(SocketException);
		Socket * acceptWithMsgLoop() throws(SocketException);
		void connectWithMsgLoop(Ipv4Address addr, word port) throws(SocketException);
		void deselectEvent() noexcept;
		Promise<void>* connectWithPromise(Ipv4Address addr, word port) noexcept;
		Promise<size_t>* readWithPromise(void * data, size_t size) noexcept;
		Promise<void>* writeWithPromise(ABuffer buffer) noexcept;
		void writeWithMsgLoop(const void * data, size_t sz) throws(SocketException, EofException, SocketWriteFailException);
		size_t readWithMsgLoop(void * data, size_t sz) throws(SocketException, EofException);
		NetworkState getState() noexcept;
		Socket * socket() noexcept;
		SocketEventHandle * event() noexcept;
		void close() noexcept;

	private:
		Socket * m_socket;
		SocketEvent m_event;
	};
}
