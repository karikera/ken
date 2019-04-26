#include "stdafx.h"
#include "wsstream.h"

using namespace kr;
using namespace ws;

char * WSStreamBase::getCursor() noexcept
{
	return m_data;
}
void WSStreamBase::setCursor(char * str) noexcept
{
	m_data = str;
}
WSIStream::WSIStream(const char * data, size_t len) noexcept
{
	m_data = (char*)data;
	m_end = (char*)data + len;
}
bool WSIStream::isEmpty() noexcept
{
	return m_data == m_end;
}
bool WSIStream::read(void * dest, size_t len) noexcept
{
	if (len == 0) return 0;
	if (m_end < m_data + len) return false;
	memcpy(dest, m_data, len);
	m_data += len;
	return true;
}
unsigned WSIStream::readInt(size_t len) noexcept
{
	char * str = m_data;
	char * end = str + len;
	if (end > m_end) return 0;
	unsigned sft = 0;
	unsigned res = 0;
	for (; str != end; str++)
	{
		res += *str << sft;
		sft += WBYTE_SHIFT;
	}
	m_data = str;

	return res;
}
float WSIStream::readFloatLP() noexcept
{
	int out = readDword() << 4;
	return (float&)out;
}
float WSIStream::readFloat() noexcept
{
	int out = readRealInt();
	return (float&)out;
}
double WSIStream::readDouble() noexcept
{
	double out;
	uint32_t* p = (uint32_t*)&out;
	p[0] = readRealInt();
	p[1] = readRealInt();
	return out;
}
unsigned WSIStream::readExtendInt() noexcept
{
	char * str = m_data;
	if (str == m_end) return 0;

	unsigned chr = *(str++);
	unsigned out = 0;
	unsigned sft = 0;
	while (chr & 0x40)
	{
		if (str == m_end) return 0;

		out |= (chr & 0x3f) << sft;
		sft += 6;

		chr = *(str++);
	}
	out |= chr << sft;
	m_data = str;
	return out;
}
size_t WSIStream::readStringNoNull(char * dest, size_t limit) noexcept
{
	size_t len = readExtendInt();
	if (len == 0) return 0;
	if (m_end < m_data + len) return 0;
	if (limit < len) limit = len;
	memcpy(dest, m_data, len);
	m_data += len;
	return len;
}
size_t WSIStream::readString(char * dest, size_t limit) noexcept
{
	size_t len = readStringNoNull(dest, limit - 1);
	dest[len] = '\0';
	return len;
}

WSOStream::WSOStream() noexcept
{
	reset();
}
void WSOStream::reset() noexcept
{
	m_data = m_buff + sizeof(WS_FRAME_EX);
}
bool WSOStream::write(const void * buff, size_t len) noexcept
{
	if (end() < m_data + len) return false;
	memcpy(m_data, buff, len);
	m_data += len;
	return true;
}
bool WSOStream::writeInt(unsigned n, size_t len) noexcept
{
	char * str = m_data;
	char * strend = str + len;
	if (strend > end())
		return false;
	for (; str != strend; str++)
	{
		*str = n & WBYTE_MASK;
		n >>= WBYTE_SHIFT;
	}
	m_data = str;
	return true;
}
bool WSOStream::writeFloatLP(float v) noexcept
{
	return writeDword((uint32_t&)v >> 4);
}
bool WSOStream::writeFloat(float v) noexcept
{
	return writeRealInt((uint32_t&)v);
}
bool WSOStream::writeDouble(double v) noexcept
{
	char * from = m_data;
	uint32_t* p = (uint32_t*)&v;
	if (!writeRealInt(p[0])) return false;
	if (!writeRealInt(p[1]))
	{
		m_data = from;
		return false;
	}
	return true;
}
bool WSOStream::writeExtendInt(dword v) noexcept
{
	char * str = m_data;
	if (str == end()) return false;

	while (v & 0xffffffC0)
	{
		*(str++) = (v & 0x3f) | 0x40;
		if (str == end()) return false;
		v >>= 6;
	}
	*(str++) = (byte)v;
	m_data = str;
	return true;
}
bool WSOStream::writeString(const char * str) noexcept
{
	size_t len = strlen(str);
	if (end() < m_data + len + 2) 
		return false;
	if (!intactTest<dword>(len))
		return false;
	writeExtendInt((dword)len);
	memcpy(m_data, str, len);
	m_data += len;
	return true;
}
size_t WSOStream::writeStream(WSIStream& stream) noexcept
{
	size_t limit = end() - m_data;
	size_t read = stream.m_end - stream.m_data;
	if (limit < read) return -1;
	memcpy(m_data, stream.m_data, read);
	stream.m_data = stream.m_end;
	m_data += read;
	return read;
}
size_t WSOStream::size() noexcept
{
	return m_data - begin();
}
char * WSOStream::begin() noexcept
{
	return m_buff + sizeof(WS_FRAME_EX);
}
char * WSOStream::end() noexcept
{
	return m_buff + sizeof(m_buff);
}
Buffer WSOStream::data() noexcept
{
	size_t datasize = size();

	WS_FRAME_EX wsf;
	memset(&wsf, 0, sizeof(wsf));
	wsf.fin = true;
	wsf.opcode = WSOpcode::Text;
	wsf.SetDataLengthAuto(datasize);

	size_t wsfsize = wsf.size();

	void * data = m_buff + sizeof(WS_FRAME_EX) - wsfsize;
	size_t sz = datasize + wsfsize;
	Buffer buff(data, sz);
	memcpy(data, &wsf, wsfsize);

	return buff;
}

