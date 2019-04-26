#include "stdafx.h"

#ifdef WIN32

#include <Ws2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

#include "mtnet.h"
#include "../msgloop.h"
#include <KR3/util/keeper.h>

using namespace kr;

enum class OperationType
{
	Accept,
	Connect,
	Read,
	Write,
};


struct kr::Operation : WSAOVERLAPPED
{
	void * owner;
	OperationType type;

	Operation(void * _owner, OperationType _type) noexcept
		:owner(_owner), type(_type)
	{
		Internal = 0;
		InternalHigh = 0;
		Offset = 0;
		OffsetHigh = 0;
		hEvent = NULL;
	}
	void reset() noexcept
	{
		memset(static_cast<WSAOVERLAPPED*>(this), 0, sizeof(WSAOVERLAPPED));
	}
};


namespace
{
	class IOCP;

	AtomicKeeper<Operation, 128> overlapes;
	GUID acceptExGuid = WSAID_ACCEPTEX;
	GUID connnectExGuid = WSAID_CONNECTEX;

	class IOCP
	{
		friend int CT_STDCALL MTServer::iocpWorker(void*) noexcept;
	private:
		Array<ThreadHandle*> m_threads;
		HANDLE m_iocp;

	public:
		IOCP() noexcept;

		Socket* createSocket(ULONG_PTR attachment);
		void close() noexcept;
		static IOCP * getInstance() noexcept;
	};

	IOCP::IOCP() noexcept
	{
		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		dword threadCount = getCPUCount() * 2;
		m_threads.resize(threadCount);

		ondebug(uint number = 1);
		for (ThreadHandle *& thread : m_threads)
		{
			ThreadId id;
			thread = ThreadHandle::createRaw<void>(&MTServer::iocpWorker, nullptr, &id);
			ondebug(id.setName(TSZ() << "MTNet " << decf(number++, 2)));
		}
	}

	void IOCP::close() noexcept
	{
		if (m_iocp)
		{
			for (ThreadHandle* worker : m_threads)
			{
				PostQueuedCompletionStatus(m_iocp, 0, 0, nullptr);
			}
			CloseHandle(m_iocp);
			m_iocp = nullptr;
		}
	}
	IOCP * IOCP::getInstance() noexcept
	{
		static IOCP iocp;
		return &iocp;
	}
	Socket* IOCP::createSocket(ULONG_PTR attachment)
	{
		if (m_iocp == nullptr) throw FunctionError("", ERROR_NOT_READY);
		SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socket == INVALID_SOCKET)
		{
			throw FunctionError("WSASocket", WSAGetLastError());
		}

		int zero = 0;
		setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));

		m_iocp = CreateIoCompletionPort((HANDLE)socket, m_iocp, attachment, 0);
		if (m_iocp == nullptr)
		{
			int errcode = GetLastError();
			closesocket(socket);
			throw FunctionError("CreateIoCompletionPort", errcode);
		}
		return (Socket*)socket;
	}

	int getSocketFunction(Socket * socket, GUID * guid, void * lpfn) noexcept
	{
		DWORD readed;
		if (WSAIoctl(
			(SOCKET)socket,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			guid,
			sizeof(GUID),
			lpfn,
			sizeof(LPFN_ACCEPTEX),
			&readed,
			nullptr,
			nullptr
		) == SOCKET_ERROR)
		{
			return WSAGetLastError();
		}
		return S_OK;
	}
}
int CT_STDCALL MTServer::iocpWorker(void*) noexcept
{
	DWORD transfered;
	ULONG_PTR attachment;
	Operation * state;
	IOCP * iocp = IOCP::getInstance();
	union
	{
		void * owner;
		MTClient * client;
		MTServer * server;
	};


	for (;;)
	{
		BOOL res = GetQueuedCompletionStatus(iocp->m_iocp, &transfered,
			&attachment,
			(LPOVERLAPPED *)&state,
			INFINITE);
		if (attachment == 0) break;
		if (state == nullptr) break;

		owner = state->owner;
		OperationType operType = state->type;

		switch (operType)
		{
		case OperationType::Read:
			if (res == 1 && transfered == 0)
			{
				client->close();
			}
			client->readCommit(res ? transfered : 0);
			break;

		case OperationType::Write:
			client->writeCommit(res ? transfered : 0);
			break;

		case OperationType::Accept:
			if (!res)
			{
				break;
			}
			server->acceptCommit(transfered);
			break;

		case OperationType::Connect:
			overlapes.free(state);
			client->onConnect();
			break;
		}
	}
	return 0;
}


MTClient::MTClient(Socket * socket) noexcept
	:m_socket(socket)
{
	AddRef();
	m_switchClient = nullptr;
	m_flushOperation = overlapes.alloc(this, OperationType::Write);
	m_receiveOperation = overlapes.alloc(this, OperationType::Read);
	m_closing = false;
	m_flushing = false;
	m_receiving = false;
	m_closed = false;
}
MTClient::MTClient() noexcept
	:MTClient(IOCP::getInstance()->createSocket(3))
{
}
MTClient::~MTClient() noexcept
{
	_assert(!m_flushing);
	_assert(!m_receiving);
	_close();
	_assert(!m_switchClient);
}
void MTClient::clear() noexcept
{
	if (m_socket)
	{
		delete m_socket;
		m_socket = nullptr;
	}
	m_receive.clear();
	m_closing = false;
	m_flushing = false;
	m_receiving = false;
}
void MTClient::reset(Socket * socket) noexcept
{
	clear();
	m_socket = socket;
}
Socket * MTClient::getSocket() noexcept
{
	return m_socket;
}
void MTClient::connect(Ipv4Address v4addr, int port) throws(SocketException)
{
	LPFN_CONNECTEX connectEx;
	int err = getSocketFunction(m_socket, &connnectExGuid, &connectEx);
	if (err) throw SocketException(err);

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	(Ipv4Address&)addr.sin_addr.s_addr = v4addr;
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	Operation * connectOperation = overlapes.alloc(this, OperationType::Connect);
	connectOperation->owner = m_socket;
	connectEx((SOCKET)socket, (sockaddr*)&addr, sizeof(addr), nullptr, 0, nullptr, connectOperation);
}
void MTClient::connect(pcstr16 host, int port) throws(SocketException)
{
	return connect(Socket::findIp(host), port);
}

void MTClient::requestReceive() noexcept
{
	{
		CsLock __lock(m_cs);
		CsLock __lock2(m_csRead);
		_assert(!m_receiving);
		if (m_switchClient)
		{
			m_switchClient->readSetBuffer(move(m_receive));

			if (!m_flushing)
			{
				m_switchClient = nullptr;
				goto __delete;
			}
			return;
		}

		if (m_closing)
		{
			if (!m_flushing)
			{
				goto __delete;
			}
			return;
		}

		auto buffer = m_receive.prepare();
		WSABUF buf;
		buf.buf = (char*)buffer.data();
		buf.len = intact<ULONG>(buffer.size());
		if (buf.len == 0)
		{
			onError("buffer prepare", ERROR_NOT_ENOUGH_MEMORY);
			close();
			if (!m_flushing)
			{
				goto __delete;
			}
			return;
		}

		m_receiving = true;
		
		DWORD flags = 0;
		DWORD received;
		m_receiveOperation->reset();
		int ret = WSARecv((SOCKET)m_socket, &buf, 1, &received, &flags, m_receiveOperation, nullptr);
		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err != WSA_IO_PENDING)
			{
				m_receiving = false;
				close();
				onError("WSARecv", err);
				if (!m_flushing)
				{
					goto __delete;
				}
				return;
			}
		}
		return;
	}
__delete:
	_close();
}
void MTClient::flush() noexcept
{
	CsLock __lock(m_cs);
	if (m_flushing) return;
	if (m_closing) return;
	if (m_writeQueue.empty()) return;

	TmpArray<WSABUF> wsabufs(0_sz, m_writeQueue.bufferCount());

	for (Buffer buf : m_writeQueue)
	{
		WSABUF * wsabuf = wsabufs.prepare(1);
		wsabuf->buf = (char*)buf.data();
		wsabuf->len = intact<DWORD>(buf.size());
	}

	m_flushing = true;

	DWORD writed;
	m_flushOperation->reset();
	int ret = WSASend((SOCKET)m_socket, wsabufs.data(), intact<DWORD>(wsabufs.size()), &writed, 0, m_flushOperation, nullptr);
	_assert(ret == SOCKET_ERROR);
	
	int err = WSAGetLastError();
	if (ERROR_IO_PENDING != err)
	{
		if (err != WSAECONNABORTED)
		{
			onError("WSASend", err);
		}
		close();
		return;
	}
}
void MTClient::write(Buffer data) noexcept
{
	writeLock();
	writeWithoutLock(data);
	writeUnlock();
}
void MTClient::writeLock() noexcept
{
	m_cs.enter();
}
void MTClient::writeWithoutLock(Buffer data) noexcept
{
	if (m_switchClient)
	{
		m_switchClient->write(data);
		return;
	}
	if (m_closing) return;

	m_writeQueue.write(data.data(), data.size());

	if (m_writeQueue.size() >= 8192)
	{
		flush();
	}
}
void MTClient::writeUnlock() noexcept
{
	m_cs.leave();
}
void MTClient::close() noexcept
{
	m_closing = true;
}
bool MTClient::isClosed() noexcept
{
	return m_closing;
}
void MTClient::readSetBuffer(BufferQueue receiver) noexcept
{
	{
		CsLock __lock(m_cs);
		if (m_closing)
		{
			if (!m_flushing)
			{
				goto __delete;
			}
			return;
		}
	}
	{
		CsLock __lock(m_csRead);
		m_receive = move(receiver);
		_callOnRead();
		requestReceive();
		return;
	}
__delete:
	_close();
}
void MTClient::readCommit(size_t size) noexcept
{
	{
		CsLock __lock(m_cs);
		_assert(m_receiving);
		m_receiving = false;

		if (m_switchClient)
		{
			m_switchClient->readSetBuffer(move(m_receive));

			if (!m_flushing)
			{
				m_switchClient = nullptr;
				close();
				goto __delete;
			}
			return;
		}

		if (m_closing)
		{
			if (!m_flushing)
			{
				close();
				goto __delete;
			}
			return;
		}
	}
	{
		CsLock __lock2(m_csRead);
		m_receive.commit(size);
		_callOnRead();
		requestReceive();
		return;
	}
__delete:
	_close();
}
void MTClient::writeCommit(size_t size) noexcept
{
	{
		CsLock __lock(m_cs);
		_assert(m_flushing);
		m_flushing = false;

		if (m_switchClient)
		{
			m_switchClient->writeCommit(size);

			if (!m_receiving)
			{
				m_switchClient = nullptr;
				_close();
			}
			return;
		}


		m_writeQueue.skip(size);

		if (m_writeQueue.empty())
		{
			onSendDone();
			if (m_closing)
			{
				if (!m_flushing)
				{
					if (m_receiving)
					{
						CancelIo(m_socket);
						// CancelIoEx(m_socket, m_receiveOperation);
					}
					else
					{
						goto __delete;
					}
				}
			}
		}
		else
		{
			flush();
		}
		return;
	}
__delete:
	_close();
}
void MTClient::callOnRead() noexcept
{
	CsLock __lock(m_csRead);
	_callOnRead();
}
void MTClient::onConnect() noexcept
{
}
void MTClient::onSendDone() noexcept
{
}
void MTClient::onClose() noexcept
{
	m_closing = true;
	m_closed = true;
	Release();
}
void MTClient::switchClient(MTClient * client) noexcept
{
	m_socket = nullptr;

	CsLock __lock(m_cs);
	m_switchClient = client;
	m_switchClient->m_flushing = true;
	client->m_writeQueue = move(m_writeQueue);
	if (m_receiving)
	{
		CancelIo(m_socket);
		// CancelIoEx(m_socket, m_receiveOperation);
	}
}

void MTClient::_close() noexcept
{
	_assert(m_closing);
	if (m_socket)
	{
		onClose();
		delete m_socket;
		m_socket = nullptr;
	}
	overlapes.free(m_receiveOperation);
	overlapes.free(m_flushOperation);
}
void MTClient::_callOnRead() noexcept
{
	if (!m_receive.empty())
	{
		try
		{
			onRead();
		}
		catch (...)
		{
		}
	}
}

MTServer::MTServer() throws(FunctionError)
{
	m_clientSocket = nullptr;
	m_serverSocket = IOCP::getInstance()->createSocket(1);
	m_acceptOperation = overlapes.alloc(this, OperationType::Accept);
}
MTServer::~MTServer() noexcept
{
	delete m_acceptOperation;
	delete m_clientSocket;
	delete m_serverSocket;
}
void MTServer::open(int port) throws(SocketException)
{
	IOCP * iocp = IOCP::getInstance();

	addrinfo hints = { 0 };
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;

	addrinfo *addrlocal = nullptr;
	if (getaddrinfo(nullptr, TSZ() << port, &hints, &addrlocal) != 0) throw SocketException();
	if (addrlocal == nullptr) throw SocketException();

	int ret = bind((SOCKET)m_serverSocket, addrlocal->ai_addr, (int)addrlocal->ai_addrlen);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		freeaddrinfo(addrlocal);
		throw SocketException(err);
	}

	ret = listen((SOCKET)m_serverSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		freeaddrinfo(addrlocal);
		throw SocketException(err);
	}

	freeaddrinfo(addrlocal);
	
	requestAccept();
}
Socket * MTServer::getClientSocket() noexcept
{
	return m_clientSocket;
}
void MTServer::requestAccept() noexcept
{
	Socket * clientSocket;
	try
	{
		IOCP * iocp = IOCP::getInstance();
		clientSocket = iocp->createSocket(2);
	}
	catch (FunctionError & err)
	{
		onError((Text)err.getFunctionName(), err);
		return;
	}
	m_clientSocket = clientSocket;

	LPFN_ACCEPTEX acceptEx;
	int err = getSocketFunction(m_serverSocket, &acceptExGuid, &acceptEx);
	if (err)
	{
		delete clientSocket;
		onError("WSAIoctl", err);
		return;
	}

	m_acceptOperation->reset();
	DWORD addressSize = sizeof(SOCKADDR_STORAGE) + 16;

	auto buf = m_receive.prepare();
	int ret = acceptEx((SOCKET)m_serverSocket, (SOCKET)clientSocket,
		buf.data(),
		intact<DWORD>(buf.size() - (2 * addressSize)),
		addressSize, addressSize,
		nullptr,
		m_acceptOperation);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			delete clientSocket;
			onError("AcceptEx", err);
			return;
		}
	}
}
void MTServer::acceptCommit(size_t size) noexcept
{
	m_clientSocket->setOption(SO_UPDATE_ACCEPT_CONTEXT, m_serverSocket);

	MTClient * client = onAccept(m_clientSocket);

	if (size)
	{
		m_receive.commit(size);
		client->readSetBuffer(move(m_receive));
	}
	else
	{
		client->requestReceive();
	}
	requestAccept();
}

#else

EMPTY_SOURCE

#endif