#include "stdafx.h"
#include "random.h"
#include <time.h>

kr::Random kr::g_random;

kr::dword kr::Random::_getExp8() noexcept
{
	dword v;
	v = math::ilog2(getDword()); if (v != -1) { return v + 95; }
	v = math::ilog2(getDword()); if (v != -1) { return v + 63; }
	v = math::ilog2(getDword()); if (v != -1) { return v + 31; }
	v = math::ilog2(getDword()) - 1;
	return ((int32_t)v > 0) * v;
}
kr::dword kr::Random::_getExp11() noexcept
{
	dword offset = 991;
	dword v;
	for (int i = 0; i< 31; i++)
	{
		v = math::ilog2(getDword());
		if (v != -1) return v + offset;
		offset -= 32;
	}
	v = math::ilog2(getDword()) - 1;
	return ((int32_t)v > 0) * v;
}

template <> void kr::Random::fill<0>(void * value) noexcept
{
}
template <> void kr::Random::fill<1>(void * value) noexcept
{
	*(byte*)value = (byte)getDword();
}
template <> void kr::Random::fill<2>(void * value) noexcept
{
	*(word*)value = (word)getDword();
}
template <> void kr::Random::fill<3>(void * value) noexcept
{
	*((word*&)value)++ = (word)getDword();
	*(byte*)value = (byte)getDword();
}
template <> void kr::Random::fill<4>(void * value) noexcept
{
	*(dword*)value = getDword();
}

inline void randfill(kr::dword * state, size_t count, kr::dword value) noexcept
{
	if (count == 0)
		return;

	kr::dword * end = state + count - 1;
	*state = value;
	while (state != end)
	{
		kr::dword next = *state * 0x176357 + 0x38562;
		state++;
		*state = next;
	}
}

kr::Random::Random() noexcept
	:Random((qword)time(nullptr) + (qword)rdtsc())
{
}
kr::Random::Random(qword seed) noexcept
{
	m_index = 0;
	randfill(m_state, 8, lodword(seed));
	randfill(m_state + 8, 8, hidword(seed));
}
kr::dword kr::Random::getDword() noexcept
{
	dword a, b, c, d;
	a = m_state[m_index];
	c = m_state[(m_index + 13) & 15];
	b = a^c ^ (a << 16) ^ (c << 15);
	c = m_state[(m_index + 9) & 15];
	c ^= (c >> 11);
	a = m_state[m_index] = b^c;
	d = a ^ ((a << 5) & 0xDA44D20);
	m_index = (m_index + 15) & 15;
	a = m_state[m_index];
	return m_state[m_index] = a^b^d ^ (a << 12) ^ (b << 18) ^ (c << 28);
}
float kr::Random::getSignedFloat() noexcept
{
	dword v = (getDword() & 0x807fffff) | (_getExp8() << 21);
	return (float&)v;
}
double kr::Random::getSignedDouble() noexcept
{
	qword v = (get<qword>() & 0x800fffffffffffffLL) | ((qword)_getExp11() << 52);
	return (double&)v;
}

template <> bool kr::Random::get<bool>() noexcept
{
	return (getDword() & 1) != 0 ? true : false;
}
template <> int64_t kr::Random::get<int64_t>() noexcept
{
	return kr::Random::get<qword>();
}
template <> uint64_t kr::Random::get<uint64_t>() noexcept
{
	return ((qword)getDword() << 32) | getDword();
}
template <> float kr::Random::get<float>() noexcept
{
	return (float)getDword() / 4294967496.0f;

	// keruch: 더 나은 랜덤 구현, 속도면에서 위쪽을 택하였다.
	//dword v = (getDword() & 0x007fffff) | (_getExp8() << 21);
	//return (float&)v;
}
template <> double kr::Random::get<double>() noexcept
{
	return (double)get<qword>() / (double)18446744073709551616.0;

	// keruch: 더 나은 랜덤 구현, 속도면에서 위쪽을 택하였다.
	//qword v = (get<qword>() & 0x000fffffffffffffLL) | ((qword)_getExp11() << 52);
	//return (double&)v;
}
template <> float kr::Random::get<float>(float value) noexcept
{
	return kr::Random::get<float>() * value;
}
template <> double kr::Random::get<double>(double value) noexcept
{
	return kr::Random::get<double>() * value;
}

void kr::Random::fill(void * dest, size_t size) noexcept
{
	char * dst = (char*)dest;
	char * end = dst + size;
	while (dst + 4 <= end) *((dword*&)dst)++ = getDword();
	if (dst + 2 <= end) *((word*&)dst)++ = (word)getDword();
	if (dst != end) *((byte*&)dst)++ = (byte)getDword();
}