#pragma once

#include <KR3/main.h>
#include <KR3/io/selfbufferedstream.h>
#include <KRUtil/net/socket.h>
#include <KRUtil/bufferqueue.h>
#include <KRUtil/serializer.h>
#include "wsevent.h"
#include "../progressor.h"
#include "../promise.h"
#include "../eventdispatcher.h"

namespace kr
{
	class Client
	{
	public:
		using Init = Socket::Init;

		Client() noexcept;
		Client(Socket* socket) noexcept;
		Client(pcstr16 host, int port) noexcept;
		~Client() noexcept;
		Client(const Client&) = delete;
		Client& operator =(const Client&) = delete;
		void connect(pcstr16 host, word port) noexcept;
		Ipv4Address getIpAddress() noexcept;
		void setSocket(Socket * socket) noexcept;
		Socket * getSocket() noexcept;
		SocketEventHandle * getSocketEvent() noexcept;
		BufferQueueWithRef * getWriteQueue() noexcept;
		void moveNetwork(Client * watcher) noexcept;
		void write(Buffer buff) noexcept;
		void writeRef(Buffer buff) noexcept;
		Promise<void>* download(Progressor * progressor, AText16 filename, size_t size) noexcept;
		void flush() noexcept;
		void close() noexcept;
		void processEvent() noexcept;
		EventProcedure makeProcedure() noexcept;

		virtual void onError(Text name, int code) noexcept = 0;
		virtual void onConnect() noexcept = 0;
		virtual void onConnectFail(int code) noexcept;
		virtual void onRead() throws(...) = 0;
		virtual void onClose() noexcept = 0;

		template <typename T>
		T serializeRead()  // EofException
		{
			Deserializer<SBISocketStream> szer = &m_stream;
			T dest;
			szer >> dest;
			return dest;
		}

		template <typename T>
		void serializeWrite(T& data) // EofException
		{
			size_t sz = SerializedSizer::getSize<T>(data);
			TBuffer buffer((size_t)0, sz);
			Serializer<TBuffer> szer = &buffer;
			szer << const_cast<T&>(data);
			write((Buffer)buffer);
		}

		template <typename T> 
		void sendPacket(T & value) noexcept
		{
			constexpr size_t opcode = SPackets::index_of<T>::value;
			static_assert(opcode <= 0xff, "packet opcode overflow");
			byte bopcode = (byte)opcode;
			serializeWrite(bopcode);
			serializeWrite(value);
		}

	protected:
		BufferQueue m_receive;

	private:
		SocketEvent m_event;
		Socket * m_socket;
		BufferQueueWithRef m_writebuf;
		bool m_waitWriteEvent;
	};


}