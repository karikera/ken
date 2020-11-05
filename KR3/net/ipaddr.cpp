#include "stdafx.h"
#include "ipaddr.h"

const kr::Ipv4Address kr::Ipv4Address::loopBack(127, 0, 0, 1);

kr::Ipv4Address::Ipv4Address(nullptr_t) noexcept
{
	*(dword*)this = 0;
}
kr::Ipv4Address::Ipv4Address(byte a, byte b, byte c, byte d) noexcept
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
};
kr::Ipv4Address::Ipv4Address(kr::Text str) noexcept
{
	uintptr_t num;
	num = str.readwith_e('.').to_uint();
	a = (byte)num;
	num = str.readwith_e('.').to_uint();
	b = (byte)num;
	num = str.readwith_e('.').to_uint();
	c = (byte)num;
	num = str.to_uint();
	d = (byte)num;
}

bool kr::Ipv4Address::operator ==(const Ipv4Address& v) const noexcept
{
	return value == v.value;
}
bool kr::Ipv4Address::operator !=(const Ipv4Address& v) const noexcept
{
	return value != v.value;
}

size_t kr::Ipv4Address::$size() const noexcept
{
	size_t sz = 3;
	for (byte v : arr)
	{
		sz += math::cipher(v, 10);
	}
	return sz;
}
template <typename CHR> size_t kr::Ipv4Address::$copyTo(CHR * dest) const noexcept
{
	ArrayWriter<CHR> writer(dest, dest + 16);
	writer << a << (CHR)'.' << b << (CHR)'.' << c << (CHR)'.' << d;
	return writer.end() - dest;
}

size_t std::hash<kr::Ipv4Address>::operator ()(const kr::Ipv4Address& ip) const noexcept
{
	return ip.value;
}

template size_t kr::Ipv4Address::$copyTo<char>(char *) const noexcept;
template size_t kr::Ipv4Address::$copyTo<char16>(char16 *) const noexcept;
template size_t kr::Ipv4Address::$copyTo<char32>(char32 *) const noexcept;
