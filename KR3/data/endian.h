
#pragma once

#include <inttypes.h>
#include "../main.h"

namespace kr
{
	template <typename T>
	T endianReverse(T value) noexcept;
}

template <typename T>
inline T kr::endianReverse(T value) noexcept
{
	T out;
	byte * p = (byte*)&n;
	byte * end = p + sizeof(T);
	byte * r = (byte*)&out + sizeof(T);
	for (; p != end; p++) *(--r) = *p;
	return out;
}

template <>
inline kr::byte kr::endianReverse<kr::byte>(byte value) noexcept
{
	return value;
}
template <>
inline kr::word kr::endianReverse<kr::word>(word value) noexcept
{
	return (value >> 8) | (value << 8);
}
