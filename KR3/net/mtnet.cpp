#include "stdafx.h"

#ifdef WIN32

#include <Ws2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

#include "mtnet.h"
#include <KR3/msg/msgloop.h>
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
	void* owner;
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
	constexpr size_t FLUSH_LIMIT = 8192;

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
		void init(DWORD threadCount) noexcept;
		bool isInited() noexcept;

		Socket* createSocket(ULONG_PTR attachment) throws(FunctionError);
		void close() noexcept;


		// 초기화 순서가 바보같이, 나중에 된다
		static IOCP* getInstance() noexcept;
	};

	IOCP::IOCP() noexcept
	{
		m_iocp = nullptr;
	}
	void IOCP::init(DWORD threadCount) noexcept
	{
		_assert(m_iocp == nullptr);
		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, threadCount);
		m_threads.resize(threadCount);

		ondebug(uint number = 1);
		for (ThreadHandle *& thread : m_threads)
		{
			ThreadId id;
			thread = ThreadHandle::createRaw<void>(&MTServer::iocpWorker, nullptr, &id);
			ondebug(id.setName(TSZ() << "MTNet " << decf(number++, 2)));
		}
	}
	bool IOCP::isInited() noexcept
	{
		return m_iocp != nullptr;
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
	Socket* IOCP::createSocket(ULONG_PTR attachment) throws(FunctionError)
	{
		_assert(m_iocp != nullptr); // need to call MTServer::init();
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

	IOCP* IOCP::getInstance() noexcept
	{
		static IOCP iocp;
		return &iocp;
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

struct MTServer::AcceptOperation : Operation
{
	Socket* m_clientSocket;
	BufferQueue m_receive;

	AcceptOperation(MTServer* server) noexcept;
	AcceptOperation(AcceptOperation&& _move) noexcept;
	~AcceptOperation() noexcept;

	void request() noexcept;
	void acceptCommit(size_t size) noexcept;
};

MTServer::AcceptOperation::AcceptOperation(MTServer* server) noexcept
	:Operation(server, OperationType::Accept)
{
}
MTServer::AcceptOperation::AcceptOperation(AcceptOperation&& _move) noexcept
	:Operation(owner, OperationType::Accept)
{
	m_clientSocket = _move.m_clientSocket;
	_move.m_clientSocket = nullptr;
	m_receive = move(_move.m_receive);
}
MTServer::AcceptOperation::~AcceptOperation() noexcept
{
	delete m_clientSocket;
}
void MTServer::AcceptOperation::request() noexcept
{
	MTServer* server = (MTServer*)owner;
	IOCP* iocp = IOCP::getInstance();
	Socket* clientSocket;
	try
	{
		clientSocket = iocp->createSocket(2);
	}
	catch (FunctionError & err)
	{
		server->onError((Text)err.getFunctionName(), err);
		return;
	}
	m_clientSocket = clientSocket;

	LPFN_ACCEPTEX acceptEx;
	int err = getSocketFunction(server->m_serverSocket, &acceptExGuid, &acceptEx);
	if (err)
	{
		delete clientSocket;
		server->onError("WSAIoctl", err);
		return;
	}

	reset();
	DWORD addressSize = sizeof(SOCKADDR_STORAGE) + 16;

	auto buf = m_receive.prepare();
	int ret = acceptEx((SOCKET)server->m_serverSocket, (SOCKET)clientSocket,
		buf.data(),
		intact<DWORD>(buf.size() - (2 * addressSize)),
		addressSize, addressSize,
		nullptr,
		this);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			delete clientSocket;
			server->onError("AcceptEx", err);
			return;
		}
	}
}
void MTServer::AcceptOperation::acceptCommit(size_t size) noexcept
{
	MTServer* server = (MTServer*)owner;
	m_clientSocket->setOption(SO_UPDATE_ACCEPT_CONTEXT, server->m_serverSocket);

	MTClient* client = server->onAccept(m_clientSocket);

	if (size)
	{
		m_receive.commit(size);
		client->readSetBuffer(move(m_receive));
	}
	else
	{
		client->requestReceive();
	}
	request();
}


int CT_STDCALL MTServer::iocpWorker(void*) noexcept
{
	DWORD transfered;
	ULONG_PTR attachment;
	Operation * state;
	union
	{
		void * owner;
		MTClient * client;
		MTServer * server;
	};

	IOCP * iocp = IOCP::getInstance();

	for (;;)
	{
		BOOL res = GetQueuedCompletionStatus(iocp->m_iocp, &transfered,
			&attachment,
			(LPOVERLAPPED *)&state,
			INFINITE);
		// res == false : socket is deleted

		if (attachment == 0) break;
		if (state == nullptr) break;

		owner = state->owner;
		OperationType operType = state->type;

		switch (operType)
		{
		case OperationType::Read:
			client->readCommit(transfered);
			break;

		case OperationType::Write:
			client->writeCommit(transfered);
			break;

		case OperationType::Accept:
			static_cast<AcceptOperation*>(state)->acceptCommit(transfered);
			break;

		case OperationType::Connect:
			overlapes.free(state);
			client->onConnect();
			break;
		}
	}
	return 0;
}

MTClient::Lock::Lock(MTClient* client) noexcept
	:m_client(client)
{
}
MTClient::Lock::~Lock() noexcept
{
	if (m_client == nullptr) return;
	if (m_client->m_writeQueue.size() >= FLUSH_LIMIT)
	{
		m_client->_flush_wl();
	}
	m_client->m_cs.leave();
}

void MTClient::Lock::write(Buffer buffer) noexcept
{
	m_client->_write_wl(buffer);
}
void MTClient::Lock::writes(View<Buffer> buffers) noexcept
{
	for (Buffer buf : buffers)
	{
		m_client->_write_wl(buf);
	}
}
void MTClient::Lock::flush() noexcept
{
	m_client->_flush_wl();
}
void MTClient::Lock::closeClient() noexcept
{
	m_client->_close_wl();
}
MTClient::Lock::operator bool() noexcept
{
	return m_client != nullptr;
}
bool MTClient::Lock::operator !() noexcept
{
	return m_client == nullptr;
}
bool MTClient::Lock::operator ==(nullptr_t) noexcept
{
	return m_client == nullptr;
}
bool MTClient::Lock::operator !=(nullptr_t) noexcept
{
	return m_client != nullptr;
}

MTClient::MTClient(Socket * socket) noexcept
	:m_socket(socket)
{
	AddRef();
	m_switchClient = nullptr;
	m_flushOperation = overlapes.alloc(this, OperationType::Write);
	m_receiveOperation = overlapes.alloc(this, OperationType::Read);
	m_closing = false;
	m_flushing = SState::Idle;
	m_receiving = RState::Processing;
}
MTClient::MTClient(MTClient* client) noexcept
	:m_socket(client->m_socket)
{
	client->m_socket = nullptr;

	AddRef();
	m_switchClient = nullptr;
	m_closing = false;

	{
		CsLock __lock(client->m_cs);
		_assert(client->m_switchClient == nullptr);
		client->m_closing = true;
		client->m_switchClient = this;
		AddRef();
		m_receiving = client->m_receiving;
		m_flushing = client->m_flushing;
		m_writeQueue = move(client->m_writeQueue);

		if (m_flushing != SState::Flushing)
		{
			if (m_flushing == SState::Processing) m_flushing = SState::Idle;
			m_flushOperation = client->m_flushOperation;
			m_flushOperation->owner = this;
			client->m_flushOperation = nullptr;
		}
		else
		{
			m_flushOperation = nullptr;
		}
		if (m_receiving != RState::Receiving)
		{
			m_receiveOperation = client->m_receiveOperation;
			m_receiveOperation->owner = this;
			client->m_receiveOperation = nullptr;
		}
		else
		{
			m_receiveOperation = nullptr;
		}
	}
	if (m_flushing != SState::Flushing && m_receiving != RState::Receiving)
	{
		Release();
		client->m_switchClient = nullptr;
		if (client->m_flushing != SState::Processing && client->m_receiving != RState::Processing)
		{
			client->m_flushing = SState::Idle;
			client->m_receiving = RState::Closed;
			client->_destroy();
		}
	}
	if (m_receiving == RState::Processing) requestReceive(); // previous clients will cancel requestReceive on Receiving
}
MTClient::MTClient() noexcept
	:MTClient(IOCP::getInstance()->createSocket(3))
{
}
MTClient::~MTClient() noexcept
{
	_assert(!m_socket);
	_assert(m_flushing == SState::Idle);
	_assert(m_receiving == RState::Closed);
	_assert(!m_switchClient);
}
void MTClient::reset(Socket * socket) noexcept
{
	delete m_socket;
	m_receive.clear();

	_assert(m_flushing == SState::Idle);
	_assert(!m_closing);
	_assert(m_receiving == RState::Processing);
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
		_assert(m_flushing != SState::Processing);
		_assert(m_receiving == RState::Processing);
		if (m_switchClient)
		{
			m_receiveOperation->owner = m_switchClient;
			m_switchClient->m_receiveOperation = m_receiveOperation;
			m_receiveOperation = nullptr;

			m_switchClient->m_receive = move(m_receive);
			m_switchClient->requestReceive();

			m_receiving = RState::Closed;
			if (m_flushing == SState::Idle)
			{
				m_switchClient->Release();
				m_switchClient = nullptr;
				goto __delete;
			}
			return;
		}

		if (m_closing)
		{
			m_receiving = RState::Closed;
			if (m_flushing == SState::Idle) goto __delete;
			return;
		}

		auto buffer = m_receive.prepare();
		WSABUF buf;
		buf.buf = (char*)buffer.data();
		buf.len = intact<ULONG>(buffer.size());
		if (buf.len == 0)
		{
			m_closing = true;
			m_receiving = RState::Closed;
			onError("buffer prepare", ERROR_NOT_ENOUGH_MEMORY);

			if (m_flushing == SState::Idle) goto __delete;
			return;
		}

		m_receiving = RState::Receiving;
		
		DWORD flags = 0;
		DWORD received;
		m_receiveOperation->reset();
		int ret = WSARecv((SOCKET)m_socket, &buf, 1, &received, &flags, m_receiveOperation, nullptr);
		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err != WSA_IO_PENDING)
			{
				m_closing = true;
				m_receiving = RState::Closed;
				onError("WSARecv", err);

				if (m_flushing == SState::Idle) goto __delete;
				return;
			}
		}
		return;
	}
	return;
__delete:
	_destroy();
}

MTClient::Lock MTClient::lock() noexcept
{
	return _writeLock() ? this : nullptr;
}
bool MTClient::isClosed() noexcept
{
	return m_closing;
}
void MTClient::readSetBuffer(BufferQueue receiver) noexcept
{
	{
		if (!m_closing)
		{
			m_receive = move(receiver);
			_callOnRead();
		}
	}

	{
		CsLock __lock(m_cs);
		_assert(m_flushing != SState::Processing);
		_assert(m_starting);
		m_starting = false;
		if (m_closing)
		{
			m_receiving = RState::Closed;
			if (m_flushing == SState::Idle) goto __delete;
			return;
		}
		requestReceive();
		return;
	}
__delete:
	_destroy();
}
void MTClient::readCommit(size_t size) noexcept
{
	{
		CsLock __lock(m_cs);
		_assert(m_flushing != SState::Processing);
		_assert(m_receiving == RState::Receiving);
		m_receiving = RState::Processing;

		if (m_switchClient)
		{
			m_receiveOperation->owner = m_switchClient;
			m_switchClient->m_receiveOperation = m_receiveOperation;
			m_receiveOperation = nullptr;

			m_switchClient->m_receive = move(m_receive);
			m_switchClient->readCommit(size);

			m_receiving = RState::Closed;
			if (m_flushing == SState::Idle)
			{
				m_switchClient->Release();
				m_switchClient = nullptr;
				goto __delete;
			}
			return;
		}
		if (m_closing)
		{
			m_receiving = RState::Closed;
			if (m_flushing == SState::Idle) goto __delete;
			return;
		}
		if (size == 0)
		{
			m_receiving = RState::Closed;
			m_closing = true;
			if (m_flushing == SState::Idle) goto __delete;
			return;
		}
	}
	m_receive.commit(size);
	_callOnRead();
	requestReceive();
	return;
__delete:
	_destroy();
}
void MTClient::writeCommit(size_t size) noexcept
{
	{
		CsLock __lock(m_cs);
		_assert(m_flushing == SState::Flushing);

		if (m_switchClient)
		{
			m_flushOperation->owner = m_switchClient;
			m_switchClient->m_flushOperation = m_flushOperation;
			m_flushOperation = nullptr;

			m_switchClient->writeCommit(size);

			if (m_receiving == RState::Closed)
			{
				m_switchClient->Release();
				m_switchClient = nullptr;
				m_flushing = SState::Idle;
				goto __delete;
			}
			return;
		}


		m_writeQueue.skip(size);

		if (m_writeQueue.empty())
		{
			m_flushing = SState::Processing;
			onSendDone();
			m_flushing = SState::Idle;
			if (m_closing)
			{
				switch (m_receiving)
				{
				case RState::Closed: goto __delete;
				case RState::Processing: break;
				case RState::Receiving:
					CancelIoEx(m_socket, m_receiveOperation);
					break;
				}
			}
		}
		else
		{
			m_flushing = SState::Idle;
			if (!m_closing)
			{
				_flush_wl();
			}
		}
		return;
	}
__delete:
	_destroy();
}
void MTClient::callOnRead() noexcept
{
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
}

bool MTClient::_writeLock() noexcept
{
	m_cs.enter();
	return m_switchClient == nullptr && !m_closing;
}

void MTClient::_destroy() noexcept
{
	_assert(m_closing);
	if (m_socket)
	{
		delete m_socket;
		m_socket = nullptr;
		onClose();
	}
	if (m_receiveOperation) overlapes.free(m_receiveOperation);
	if (m_flushOperation) overlapes.free(m_flushOperation);
	Release();
}
void MTClient::_callOnRead() noexcept
{
	if (!m_receive.empty())
	{
		try
		{
			onRead();
		}
		catch (ThrowAbort&)
		{
			lock().closeClient();
		}
		catch (...)
		{
		}
	}
}
void MTClient::_write_wl(Buffer buffer) noexcept
{
	m_writeQueue.write(buffer.data(), buffer.size());
}
void MTClient::_flush_wl() noexcept
{
	if (m_flushing == SState::Flushing) return;
	if (m_writeQueue.empty()) return;
	m_flushing = SState::Flushing;

	TmpArray<WSABUF> wsabufs;
	wsabufs.reserve(m_writeQueue.bufferCount());

	for (Buffer buf : m_writeQueue)
	{
		WSABUF* wsabuf = wsabufs.prepare(1);
		wsabuf->buf = (char*)buf.data();
		wsabuf->len = intact<DWORD>(buf.size());
	}

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
		_close_wl();
		return;
	}
}
void MTClient::_close_wl() noexcept
{
	_assert(!m_closing);
	m_closing = true;
	if (m_flushing == SState::Idle && m_receiving == RState::Receiving)
	{
		CancelIoEx(m_socket, m_receiveOperation);
	}
}


void MTServer::init(uint threadCount) noexcept
{
	IOCP* iocp = IOCP::getInstance();
	if (iocp->isInited()) return;
	iocp->init(threadCount);
}
void MTServer::init() noexcept
{
	IOCP* iocp = IOCP::getInstance();
	if (iocp->isInited()) return;
	iocp->init(getCPUCount()*2);
}
bool MTServer::isInited() noexcept
{
	IOCP* iocp = IOCP::getInstance();
	return iocp->isInited();
}
MTServer::MTServer() throws(FunctionError)
{
	init();
	IOCP* iocp = IOCP::getInstance();

	m_serverSocket = iocp->createSocket(1);
}
MTServer::~MTServer() noexcept
{
	m_acceptOperations = nullptr;
	delete m_serverSocket;
}
void MTServer::open(int port) throws(SocketException)
{
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

	m_acceptOperations.initResize(8, this);

	ret = listen((SOCKET)m_serverSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		freeaddrinfo(addrlocal);
		throw SocketException(err);
	}

	freeaddrinfo(addrlocal);
	
	for (AcceptOperation& accept : m_acceptOperations)
	{
		accept.request();
	}
}

#else

EMPTY_SOURCE

#endif