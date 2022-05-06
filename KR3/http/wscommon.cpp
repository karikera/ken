#include "stdafx.h"
#include "wscommon.h"

using namespace kr;

uint16_t kr::endian::reverse(uint16_t v) noexcept
{
	return kr::intrinsic<2>::rotl(v, 8);
}
uint32_t kr::endian::reverse(uint32_t v) noexcept
{
	return (v << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | (v >> 24);
}
uint64_t kr::endian::reverse(uint64_t v) noexcept
{
	return (v << 56) |
		((v & 0xff00) << 40) |
		((v & 0xff0000) << 24) |
		((v & 0xff000000) << 8) |
		((v >> 8) & 0xff000000) |
		((v >> 24) & 0xff0000) |
		((v >> 40) & 0xff00) |
		(v >> 56);
}

TText kr::makeSecWebSocketAccept(Text key) noexcept
{
	static const char WS_GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	BText<128> keys;
	keys << key;
	keys << WS_GUID;

	return (TText)(encoder::Base64)(BBuffer<20>)(encoder::Sha1)keys;
}


size_t kr::WSFrame::getLengthExtend() noexcept
{
	if (length < 126) return 0;
	if (length == 126) return 2;
	if (length == 127) return 8;
	_assert(!"Invalid length");
	return 0;
}
size_t kr::WSFrame::getMaskExtend() noexcept
{
	return (mask ? 4 : 0);
}
size_t kr::WSFrame::getExtendSize() noexcept
{
	return getLengthExtend() + getMaskExtend();
}
size_t kr::WSFrame::getSize() noexcept
{
	return sizeof(WSFrame) + getExtendSize();
}
uint64_t kr::WSFrame::getDataLength() noexcept
{
	if (length < 126) return length;
	if (length == 126) return endian::reverse(*(uint16_t*)(this + 1));
	if (length == 127) return endian::reverse(*(uint64_t*)(this + 1));
	_assert(!"Invalid length");
	return 0;
}
uint32_t kr::WSFrame::getMask() noexcept
{
	return *(Unaligned<uint32_t>*)((char*)(this + 1) + getLengthExtend());
}
void kr::WSFrame::setMask(uint32_t value) noexcept
{
	mask = true;
	*(Unaligned<uint32_t>*)((char*)(this + 1) + getLengthExtend()) = value;
}
void* kr::WSFrame::getData() noexcept
{
	return ((char*)(this + 1) + getExtendSize());
}
void kr::WSFrame::setDataLengthAuto(uint64_t len) noexcept
{
	if (len < 126)
	{
		setDataLength7((uint8_t)len);
	}
	else if (len < 65536)
	{
		setDataLength16((uint16_t)len);
	}
	else
	{
		setDataLength64(len);
	}
}
void kr::WSFrame::setDataLength7(uint8_t len) noexcept
{
	length = len;
}
void kr::WSFrame::setDataLength16(uint16_t len)
{
	length = 126;
	*(uint16_t*)(this + 1) = endian::reverse((uint16_t)len);
}
void kr::WSFrame::setDataLength64(uint64_t len)
{
	length = 127;
	*(uint64_t*)(this + 1) = endian::reverse(len);
}

WSFrameReader::WSFrameReader() noexcept
{
	m_state = State::Frame;
}
Buffer WSFrameReader::readFrom(BufferQueue* receive) throws(EofException, ThrowAbort, TooBigException)
{
	switch (m_state)
	{
	case State::ClearData:{
		data.clear();
		m_state = State::Frame;
		[[fallthrough]];
	}
	case State::Frame: {
	_parseFrame:
		receive->read(&frame, sizeof(WSFrame));
		m_state = State::FrameEx;
		[[fallthrough]];
	}
	case State::FrameEx: {
		size_t needex = frame.getExtendSize();
		receive->read((char*)&frame + sizeof(WSFrame), needex);
		switch (frame.opcode)
		{
		case WSOpcode::CONTINUE: break;
		case WSOpcode::CLOSE:
			opcode = frame.opcode;
			throw ThrowAbort();
		case WSOpcode::PING:
		case WSOpcode::PONG:
		case WSOpcode::BINARY:
		case WSOpcode::TEXT:
			opcode = frame.opcode;
			break;
		}
		m_state = State::Data;
		[[fallthrough]];
	}
	case State::Data: {
		uint64_t datalen64 = frame.getDataLength();
		if (datalen64 >= DATA_SIZE_LIMIT)
		{
			throw TooBigException();
		}
		size_t datalen = (size_t)datalen64;

		receive->read(datalen).readAllTo(&data);

		if (frame.mask)
		{
			mem::xor_copy((byte*)data.data(), data.data(), datalen, frame.getMask());
		}

		if (frame.fin)
		{
			m_state = State::ClearData;
		}
		else
		{
			m_state = State::Frame;
			goto _parseFrame;
		}
		return data;
	}
	default:
		unreachable();
	}
}
