#pragma once

#include <KR3/main.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/net/socket.h>
#include <KR3/util/bufferqueue.h>
#include <KR3/util/serializer.h>
#include <KR3/msg/progressor.h>
#include <KR3/msg/promise.h>
#include "wsevent.h"

namespace kr
{
	template <typename T>
	struct GetPacketId
	{
		static constexpr size_t value = T::ID;
	};

	class Client
	{
	public:
		using Init = Socket::Init;

		class Lock
		{
			friend Client;
		public:
			~Lock() noexcept;

			void write(Buffer buffer) noexcept;
			void flush() noexcept;

			operator bool() noexcept;
			bool operator !() noexcept;
			bool operator ==(nullptr_t) noexcept;
			bool operator !=(nullptr_t) noexcept;

		private:
			Lock(Client * client) noexcept;
			Client* m_client;
		};

		Client() noexcept;
		Client(Socket* socket) noexcept;
		Client(pcstr16 host, int port) throws(SocketException);
		~Client() noexcept;
		Client(const Client&) = delete;
		Client& operator =(const Client&) = delete;
		void connect(pcstr16 host, word port) throws(SocketException);
		void connectSync(pcstr16 host, word port) throws(SocketException);
		Ipv4Address getIpAddress() noexcept;
		void setSocket(Socket * socket) noexcept;
		Socket * getSocket() noexcept;
		SocketEventHandle * getSocketEvent() noexcept;
		BufferQueue* getWriteQueue() noexcept;

		void moveNetwork(Client * watcher) noexcept;

		// for compatibility with MTClient
		// Actually, It does not lock
		Lock lock() noexcept;

		void write(Buffer buff) noexcept;
		Promise<void>* download(Progressor * progressor, AText16 filename, size_t size) noexcept;
		void flush() noexcept;
		void close() noexcept;
		void processEvent() noexcept;
		EventProcedure makeProcedure() noexcept;

		virtual void onError(Text name, int code) noexcept;
		virtual void onConnect() noexcept;
		virtual void onConnectFail(int code) noexcept;
		virtual void onRead() throws(...) = 0;
		virtual void onClose() noexcept;
		virtual void onWriteBegin() noexcept;
		virtual void onWriteEnd() noexcept;

		template <typename T>
		T serializeRead()  // EofException
		{
			Deserializer<SBISocketStream> szer = &m_receive;
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
			constexpr size_t opcode = GetPacketId<T>::value;
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
		BufferQueue m_writebuf;
		bool m_waitWriteEvent;
	};


}