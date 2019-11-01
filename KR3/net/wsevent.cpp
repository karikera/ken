#include "stdafx.h"

#ifdef WIN32

#include "wsevent.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <KR3/net/socket.h>
#include <KR3/msg/pump.h>
#include <KR3/msg/eventdispatcher.h>

using namespace kr;

static_assert(FD_MAX_EVENTS == NetworkState::maxEventCount, "FD_MAX_EVENTS unmatch");

FNetworkEvent::FNetworkEvent() noexcept
{
}
FNetworkEvent FNetworkEvent::getDefaultClient() noexcept
{
	return FNetworkEvent(true, true, false, true, false);
}
FNetworkEvent FNetworkEvent::getDefaultServer() noexcept
{
	return FNetworkEvent(false, false, false, false, true);
}
FNetworkEvent::FNetworkEvent(bool _write, bool _read, bool _connect, bool _close, bool _accept) noexcept
{
	raw(*this) = 0;
	write = _write;
	read = _read;
	connect = _connect;
	close = _close;
	accept = _accept;
}

SocketEventHandle* SocketEventHandle::create() noexcept
{
	WSAEVENT hEvent = WSACreateEvent();
	_assert(hEvent != WSA_INVALID_EVENT);
	return (SocketEventHandle*)hEvent;
}
void SocketEventHandle::operator delete(ptr p) noexcept
{
	WSACloseEvent((WSAEVENT)p);
}
int SocketEventHandle::select(Socket* sock, FNetworkEvent lNetworkEvents) noexcept
{
	return WSAEventSelect((SOCKET)sock, this, (long&)lNetworkEvents);
}
int SocketEventHandle::deselect(Socket * sock) noexcept
{
	return WSAEventSelect((SOCKET)sock, nullptr, 0);
}
NetworkState SocketEventHandle::getState(Socket* sock) noexcept
{
	WSANETWORKEVENTS msg;
	int enumNetowkrEventsResult = WSAEnumNetworkEvents((SOCKET)sock, this, &msg);
	if (enumNetowkrEventsResult != 0)
	{
		debug();
		mema::zero(msg);
	}
	return (NetworkState&)msg;
}
Promise<void> * SocketEventHandle::waitState(Socket* sock, int fdbit) noexcept
{
	DeferredPromise<void> * prom = new DeferredPromise<void>;
	EventDispatcher::registThreaded(this, [this, prom, sock, fdbit](DispatchedEvent * ev) {
		try
		{
			NetworkState ne = getState(sock);
			if (ne.bits & (1 << fdbit))
			{
				int error = ne.errors[fdbit];
				if (error == 0)
				{
					ev->detach();
					prom->_resolve();
				}
				else
				{
					throw SocketException(error);
				}
			}
		}
		catch (...)
		{
			ev->detach();
			prom->_reject();
		}
	});
	return prom;
}
Promise<NetworkState> * SocketEventHandle::waitStates(Socket* sock, int fdmask) noexcept
{
	DeferredPromise<NetworkState> * prom = new DeferredPromise<NetworkState>;
	EventDispatcher::registThreaded(this, [this, prom, sock, fdmask](DispatchedEvent * ev) {
		try
		{
			NetworkState ne = getState(sock);
			if (ne.bits & fdmask)
			{
				ev->detach();
				prom->_resolve(ne);
			}
		}
		catch (...)
		{
			ev->detach();
			prom->_reject();
		}
	});
	return prom;
}

void SocketEventHandle::set() noexcept
{
	WSASetEvent(this);
}
void SocketEventHandle::reset() noexcept
{
	WSAResetEvent(this);
}

_pri_::SocketEventListImpl::SocketEventListImpl() noexcept
{
}
SocketEventHandle * _pri_::SocketEventListImpl::pickOut(size_t idx) noexcept
{
	m_size--;
	if (idx == m_size) return _ptr()[idx];
	SocketEventHandle * out = _ptr()[idx];
	_ptr()[idx] = _ptr()[m_size];
	return out;
}
SocketEventHandle * _pri_::SocketEventListImpl::get(size_t idx) noexcept
{
	_assert(idx < m_size);
	return _ptr()[idx];
}
SocketEventHandle ** _pri_::SocketEventListImpl::_ptr() noexcept
{
	return (SocketEventHandle**)(this + 1);
}

SocketEvent::SocketEvent(SocketEventHandle * handle) noexcept
{
	m_event = handle;
}
SocketEvent::SocketEvent() noexcept
{
	m_event = SocketEventHandle::create();
}
SocketEvent::SocketEvent(SocketEvent && move) noexcept
{
	m_event = move.m_event;
	move.m_event = nullptr;
}
SocketEvent::~SocketEvent() noexcept
{
	if (m_event != nullptr) delete m_event;
}

SocketEventHandle * SocketEvent::operator ->() noexcept
{
	_assert(m_event != nullptr);
	return m_event;
}
SocketEventHandle * SocketEvent::operator &() noexcept
{
	return m_event;
}
SocketEvent & SocketEvent::operator = (SocketEvent && move) noexcept
{
	this->~SocketEvent();
	m_event = move.m_event;
	move.m_event = nullptr;
	return *this;
}

EventedSocket::EventedSocket() noexcept
{
	m_socket = nullptr;
}
EventedSocket::~EventedSocket() noexcept
{
	delete m_socket;
}

void EventedSocket::open(word port) throws(SocketException)
{
	delete m_socket;
	m_socket = Socket::create();

	m_event->select(m_socket, FNetworkEvent(false, false, false, false, true));
	m_event->reset();
	m_socket->open(port);
}
Socket * EventedSocket::acceptWithMsgLoop() throws(SocketException)
{
	EventPump::getInstance()->wait(&m_event);
	NetworkState state = m_event->getState(m_socket);
	if (state.errAccept != 0) throw SocketException(state.errAccept);
	return m_socket->accept();
}
void EventedSocket::connectWithMsgLoop(Ipv4Address addr, word port) throws(SocketException)
{
	delete m_socket;
	m_socket = Socket::create();

	m_event->select(m_socket, FNetworkEvent(false, false, true, false, false));
	m_event->reset();
	m_socket->connectAsync(addr, port);

	EventPump * pump = EventPump::getInstance();
	pump->wait(&m_event);

	NetworkState state = m_event->getState(m_socket);
	if (state.errConnect != 0) throw SocketException(state.errConnect);
}
void EventedSocket::deselectEvent() noexcept
{
	m_event->deselect(m_socket);
}
Promise<void>* EventedSocket::connectWithPromise(Ipv4Address addr, word port) noexcept
{
	try
	{
		delete m_socket;
		m_socket = Socket::create();

		m_event->select(m_socket, FNetworkEvent(false, false, true, false, false));
		m_event->reset();
		m_socket->connectAsync(addr, port);

		return m_event->waitState(m_socket, FD_CONNECT_BIT)->then([this] {
			m_event->select(m_socket, FNetworkEvent(false, true, false, true, false));
		});
	}
	catch (...)
	{
		return Promise<void>::reject();
	}
}
Promise<size_t>* EventedSocket::readWithPromise(void * data, size_t size) noexcept
{
	DeferredPromise<size_t> * prom = new DeferredPromise<size_t>;
	if (size == 0)
	{
		prom->_resolve(0);
		return prom;
	}

	try
	{
		if (m_socket == nullptr) throw EofException();
		m_event->select(m_socket, FNetworkEvent(false, true, false, true, false));
		size_t left = size;
		EventDispatcher::registThreaded(&m_event, [this, prom, data, size, left](DispatchedEvent * ev) mutable {
			try
			{
				NetworkState ne = getState();
				if (ne.read)
				{
					size_t readed = m_socket->$read(data, left);
					(byte*&)data += readed;
					left -= readed;
					if (left == 0)
					{
						ev->detach();
						prom->_resolve(size);
					}
				}
				if (ne.close)
				{
					delete m_socket;
					m_socket = nullptr;
					ev->detach();
					prom->_resolve(size - left);
				}
			}
			catch (...)
			{
				ev->detach();
				prom->_reject();
			}
		});
	}
	catch (...)
	{
		prom->_reject();
	}
	return prom;
}
Promise<void>* EventedSocket::writeWithPromise(ABuffer buffer) noexcept
{
	DeferredPromise<void> * prom = new DeferredPromise<void>;
	if (buffer.empty())
	{
		prom->_resolve();
		return prom;
	}

	try
	{
		if (m_socket == nullptr) throw EofException();

		m_event->select(m_socket, FNetworkEvent(false, true, false, true, false));
		try
		{
			m_socket->$write(buffer.data(), buffer.size());
		}
		catch (ThrowAbort&)
		{
			EventDispatcher::registThreaded(&m_event, [this, prom, buffer = move(buffer)](DispatchedEvent * ev) mutable {
				try
				{
					NetworkState ne = getState();
					if (ne.close)
					{
						delete m_socket;
						m_socket = nullptr;
						throw SocketWriteFailException(buffer);
					}
					if (ne.write)
					{
						try
						{
							m_socket->$write(buffer.data(), buffer.size());
							ev->detach();
							prom->_resolve();
						}
						catch (ThrowAbort&)
						{
						}
					}
				}
				catch (...)
				{
					ev->detach();
					prom->_reject();
				}
			});
		}
	}
	catch (...)
	{
		prom->_reject();
	}
	return prom;
}
void EventedSocket::writeWithMsgLoop(const void * data, size_t sz) throws(SocketException, EofException, SocketWriteFailException)
{
	if (sz == 0) return;
	if (m_socket == nullptr) throw EofException();

	m_event->select(m_socket, FNetworkEvent(true, false, false, true, false));
	try
	{
		m_socket->$write(data, sz);
	}
	catch (ThrowAbort&)
	{
		EventPump * pump = EventPump::getInstance();
		for (;;)
		{
			pump->wait(&m_event);
			NetworkState state = m_event->getState(m_socket);
			if (state.close)
			{
				delete m_socket;
				m_socket = nullptr;
				throw SocketWriteFailException(Buffer(data, sz));
			}
			if (state.write)
			{
				m_socket->$write(data, sz);
				return;
			}
		}
	}
}
size_t EventedSocket::readWithMsgLoop(void * data, size_t sz) throws(SocketException, EofException)
{
	if (sz == 0) return 0;
	if (m_socket == nullptr) throw EofException();

	size_t left = sz;
	m_event->select(m_socket, FNetworkEvent(false, true, false, true, false));
	EventPump * pump = EventPump::getInstance();

	for (;;)
	{
		pump->wait(&m_event);
		NetworkState state = m_event->getState(m_socket);

		if (state.read)
		{
			size_t readed = m_socket->$read(data, left);
			left -= readed;
			(byte*&)data += readed;
			if (left == 0) return sz;
		}
		if (state.close)
		{
			delete m_socket;
			m_socket = nullptr;
			return sz - left;
		}
	}
}
NetworkState EventedSocket::getState() noexcept
{
	return m_event->getState(m_socket);
}
Socket * EventedSocket::socket() noexcept
{
	return m_socket;
}
SocketEventHandle * EventedSocket::event() noexcept
{
	return &m_event;
}
void EventedSocket::close() noexcept
{
	delete m_socket;
	m_socket = nullptr;
}


#else

EMPTY_SOURCE

#endif