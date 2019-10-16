#include "stdafx.h"
#ifdef WIN32
#include "client.h"

#include "../pool.h"

#include <KR3/net/socket.h>
#include <WinSock2.h>

using namespace kr;

Client::Client() noexcept
{
	m_socket = nullptr;
	m_waitWriteEvent = true;
}
Client::Client(Socket * socket) noexcept
	:Client()
{
	m_socket = socket;
	m_event->select(socket, FNetworkEvent(true, true, true, true, false));
}
Client::Client(pcstr16 host, int port) throws(SocketException)
	:Client()
{
	connect(host, port);
}
Client::~Client() noexcept
{
	delete m_socket;
}
void Client::connect(pcstr16 host, word port) throws(SocketException)
{
	close();

	m_socket = Socket::create();
	m_event->select(m_socket, FNetworkEvent(true, true, true, true, false));
	m_socket->connectAsync(Socket::findIp(host), port);
}
Ipv4Address Client::getIpAddress() noexcept
{
	return m_socket->getIpAddress();
}
void Client::setSocket(Socket * socket) noexcept
{
	m_socket = socket;
}
Socket * Client::getSocket() noexcept
{
	return m_socket;
}
SocketEventHandle * Client::getSocketEvent() noexcept
{
	return &m_event;
}
void Client::moveNetwork(Client * watcher) noexcept
{
	m_socket = watcher->m_socket;
	watcher->m_socket = nullptr;
	m_writebuf = move(watcher->m_writebuf);
	m_receive = move(watcher->m_receive);
}
void Client::write(Buffer buff) noexcept
{
	m_writebuf.write(buff.begin(), buff.size());
}
void Client::writeRef(Buffer buff) noexcept
{
	m_writebuf.writeRef(buff.begin(), buff.size());
}
BufferQueueWithRef * Client::getWriteQueue() noexcept
{
	return &m_writebuf;
}
void Client::flush() noexcept
{
	if (m_waitWriteEvent) return;
	try
	{
		while (!m_writebuf.empty())
		{
			Buffer buffer = m_writebuf.getFirstBlock();
			try
			{
				size_t sz = buffer.size();
				m_socket->writeImpl(buffer.data(), sz);
				m_writebuf.skip(sz);
			}
			catch (ThrowAbort&)
			{
				m_waitWriteEvent = true;
				break;
			}
		}
	}
	catch (SocketException& err)
	{
		onError("flush", err);
	}
}
void Client::close() noexcept
{
	delete m_socket;
	m_socket = nullptr;
}
void Client::processEvent() noexcept
{
	NetworkState state = m_event->getState(m_socket);
	if (state.connect)
	{
		if (state.errConnect)
		{
			onConnectFail(state.errConnect);
		}
		else
		{
			onConnect();
		}
	}
	if (state.read)
	{
		if (state.errRead)
		{
			onError("read", state.errRead);
		}
		else
		{
			try
			{
				for (;;)
				{
					{
						auto buf = m_receive.prepare();
						size_t readed = m_socket->readImpl(buf.data(), buf.size());
						if (readed == 0) break;
						m_receive.commit(readed);
					}
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
			}
			catch (EofException&)
			{
			}
			catch (SocketException& err)
			{
				onError("read", err);
			}
		}
	}
	if (state.write)
	{
		if (state.errWrite)
		{
			onError("write", state.errWrite);
		}
		else
		{
			try
			{
				try
				{
					while (!m_writebuf.empty())
					{
						Buffer block = m_writebuf.getFirstBlock();
						size_t size = block.size();
						m_socket->writeImpl((char*)block.data(), size);
						m_writebuf.skip(size);
					}
					m_waitWriteEvent = false;
				}
				catch (ThrowAbort&)
				{
				}
			}
			catch (SocketException& err)
			{
				onError("write", err);
			}
		}
	}
	if (state.close)
	{
		// state.errClose
		onClose();
	}
}
EventProcedure Client::makeProcedure() noexcept
{
	EventProcedure proc;
	proc.callback = [](void * param) {
		((Client*)param)->processEvent();
	};
	proc.event = &m_event;
	proc.param = this;
	return proc;
}
void Client::onConnectFail(int code) noexcept
{
}

constexpr size_t TEMP_BUFFER = 8192;

#ifdef NO_USE_FILESYSTEM

Promise<void>* Client::download(Progressor * progressor, const char16 * filename, size_t size) noexcept
{
	return Promise<void>::reject(UnsupportedException());
}

#else

#include <KR3/fs/file.h>

Promise<void>* Client::download(Progressor * progressor, AText16 filename, size_t size) noexcept
{
	filename.c_str();
	m_event->deselect(m_socket);

	return threading([this, progressor, filename = move(filename), size] {
		try
		{
			finally{
				m_event->select(m_socket, FNetworkEvent(true, true, true, true, false));
			};
			Must<File> file = File::create(filename.data());
			progressor->setProgress(0, size);
			progressor->checkPoint();

			{
				for (Buffer buf : m_receive)
				{
					file->writeImpl(buf.data(), buf.size());
				}
				m_receive.clear();
			}

			size_t sz = size;
			TmpArray<char> buffer(TEMP_BUFFER);
			while (sz != 0)
			{
				size_t r = m_socket->readImpl(buffer.data(), mint((size_t)TEMP_BUFFER, sz));
				if (r == 0) continue;

				file->writeImpl(buffer.data(), r);
				sz -= r;
				progressor->checkPoint(progressor->getProgress() + r);
				progressor->onProgress();
			}
		}
		catch (...)
		{
		}
	});
}

#endif


#else

EMPTY_SOURCE

#endif