#include "stdafx.h"
#include "math.h"

#ifdef _MSC_VER
#include <intrin.h>

#pragma intrinsic(sqrt,fabs,sin,cos,tan)
#pragma optimize("ts",on)

#endif

// [standard cast] : too fast
// fld fistp: 3.0c
// lround(x): 63.5c
// (int)round(x): 59.5c

// (int)sqrt((float)x): 11.0c
// isqrt(100) : 19.8c

// tan: 25.9c
// asm tan: 176.7c 

#define _RETURN_	ATTR_NO_DISCARD ATTR_NOALIAS

template <typename T>
kr::byte adc_impl(kr::byte carry, T v1, T v2, void * res)
{
	v1 = v1 + v2 + carry;
	*(T*)res = v1;
	return (v1 < v2);
}
template <typename T>
kr::byte sbb_impl(kr::byte borrow, T v1, T v2, void * res)
{
	v2 = v1 - v2 - borrow;
	*(T*)res = v2;
	return (v2 > v1);
}


_RETURN_ kr::byte kr::intrinsic<1>::adc(byte carry, byte v1, byte v2, void* res) noexcept
{
#ifdef _MSC_VER
	return _addcarry_u8(carry,v1,v2,(byte*)res);
#else
	return adc_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<2>::adc(byte carry, word v1, word v2, void* res) noexcept
{
#ifdef _MSC_VER
	return _addcarry_u16(carry,v1,v2,(word*)res);
#else
	return adc_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<3>::adc(byte carry, dword v1, dword v2, void* res) noexcept
{
	carry = intrinsic<2>::adc(carry,(word)v1, (word)v2, res);
	return intrinsic<1>::adc(carry, (byte)(v1>>16), (byte)(v2>>16), (word*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<4>::adc(byte carry, dword v1, dword v2, void* res) noexcept
{
#ifdef _MSC_VER
	return _addcarry_u32(carry, v1, v2, (uint*)res);
#else
	return adc_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<5>::adc(byte carry, qword v1, qword v2, void* res) noexcept
{
	carry = intrinsic<4>::adc(carry,(dword)v1, (dword)v2, res);
	return intrinsic<1>::adc(carry, (byte)(v1>>32), (byte)(v2>>32), (dword*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<6>::adc(byte carry, qword v1, qword v2, void* res) noexcept
{
	carry = intrinsic<4>::adc(carry,(dword)v1, (dword)v2, res);
	return intrinsic<2>::adc(carry, (word)(v1>>32), (word)(v2>>32), (dword*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<7>::adc(byte carry, qword v1, qword v2, void* res) noexcept
{
	carry = intrinsic<4>::adc(carry,(dword)v1, (dword)v2, res);
	return intrinsic<3>::adc(carry, (dword)(v1>>32), (dword)(v2>>32), (dword*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<8>::adc(byte carry, qword v1, qword v2, void* res) noexcept
{
#ifdef _MSC_VER
#ifdef _M_X64
	return _addcarry_u64(carry, v1, v2, (qword*)res);
#else
	carry = intrinsic<4>::adc(carry, (dword)v1, (dword)v2, res);
	return intrinsic<4>::adc(carry, (dword)(v1 >> 32), (dword)(v2 >> 32), (dword*)res + 1);
#endif
#else
	return adc_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<1>::sbb(byte carry, byte v1, byte v2, void* res) noexcept
{
#ifdef _MSC_VER
	return _subborrow_u8(carry,v1,v2,(byte*)res);
#else
	return sbb_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<2>::sbb(byte carry, word v1, word v2, void* res) noexcept
{
#ifdef _MSC_VER
	return _subborrow_u16(carry,v1,v2,(word*)res);
#else
	return sbb_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<3>::sbb(byte carry, dword v1, dword v2, void* res) noexcept
{
	carry = intrinsic<2>::sbb(carry,(word)v1, (word)v2, res);
	return intrinsic<1>::sbb(carry, (byte)(v1>>16), (byte)(v2>>16), (word*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<4>::sbb(byte carry, dword v1, dword v2, void* res) noexcept
{
#ifdef _MSC_VER
	return _subborrow_u32(carry,v1,v2,(uint*)res);
#else
	return sbb_impl(carry, v1, v2, res);
#endif
}
_RETURN_ kr::byte kr::intrinsic<5>::sbb(byte carry, qword v1, qword v2, void* res) noexcept
{
	carry = intrinsic<4>::adc(carry,(dword)v1, (dword)v2, res);
	return intrinsic<1>::adc(carry, (byte)(v1>>32), (byte)(v2>>32), (dword*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<6>::sbb(byte carry, qword v1, qword v2, void* res) noexcept
{
	carry = intrinsic<4>::sbb(carry,(dword)v1, (dword)v2, res);
	return intrinsic<2>::sbb(carry, (word)(v1>>32), (word)(v2>>32), (dword*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<7>::sbb(byte carry, qword v1, qword v2, void* res) noexcept
{
	carry = intrinsic<4>::sbb(carry,(dword)v1, (dword)v2, res);
	return intrinsic<3>::sbb(carry, (dword)(v1>>32), (dword)(v2>>32), (dword*)res + 1);
}
_RETURN_ kr::byte kr::intrinsic<8>::sbb(byte carry, qword v1, qword v2, void* res) noexcept
{
#ifdef _MSC_VER
#ifdef _M_X64
	return _subborrow_u64(carry, v1, v2, (qword*)res);
#else
	carry = intrinsic<4>::sbb(carry, (dword)v1, (dword)v2, res);
	return intrinsic<4>::sbb(carry, (dword)(v1 >> 32), (dword)(v2 >> 32), (dword*)res + 1);
#endif
#else
	return sbb_impl(carry, v1, v2, res);
#endif
}

_RETURN_ kr::word kr::intrinsic<2>::rotl(word value, byte shift) noexcept
{
#ifdef _MSC_VER
	return _rotl16(value, shift);
#elif defined(__GNUG__) && !defined(__EMSCRIPTEN__)
	asm("roll %1,%0" : "+r"(value) : "r"(shift));
	return value;
#else
	shift %= 16;
	return (value << shift) | (value >> (16 - shift));
#endif
}
_RETURN_ kr::word kr::intrinsic<2>::rotr(word value, byte shift) noexcept
{
#ifdef _MSC_VER
	return _rotr16(value, shift);
#elif defined(__GNUG__) && !defined(__EMSCRIPTEN__)
	asm("rorl %1,%0" : "+r" (value) : "r" (shift));
	return value;
#else
	shift %= 16;
	return (value >> shift) | (value << (16 - shift));
#endif
}
_RETURN_ kr::dword kr::intrinsic<4>::rotl(dword value, int shift) noexcept
{
#ifdef _MSC_VER
	return _rotl(value, shift);
#elif defined(__GNUG__) && !defined(__EMSCRIPTEN__)
	asm ("roll %1,%0" : "+r"(value) : "r"(shift));
	return value;
#else
	shift %= 32;
	return (value << shift) | (value >> (32 - shift));
#endif
}
_RETURN_ kr::dword kr::intrinsic<4>::rotr(dword value, int shift) noexcept
{
#ifdef _MSC_VER
	return _rotr(value, shift);
#elif defined(__GNUG__) && !defined(__EMSCRIPTEN__)
	asm("rorl %1,%0" : "+r" (value) : "r" (shift));
	return value;
#else
	shift %= 32;
	return (value >> shift) | (value << (32-shift));
#endif
}
_RETURN_ kr::qword kr::intrinsic<8>::rotl(qword value, int shift) noexcept
{
#ifdef _MSC_VER
	return _rotl64(value, shift);
#elif defined(__GNUG__) && !defined(__EMSCRIPTEN__)
	asm("rol64l %1,%0" : "+r" (value) : "r" (shift));
	return value;
#else
	shift %= 64;
	return (value << shift) | (value >> (64 - shift));
#endif
}
_RETURN_ kr::qword kr::intrinsic<8>::rotr(qword value, int shift) noexcept
{
#ifdef _MSC_VER
	return _rotr64(value, shift);
#elif defined(__GNUG__) && !defined(__EMSCRIPTEN__)
	asm("ror64l %1,%0" : "+r" (value) : "r" (shift));
	return value;
#else
	shift %= 64;
	return (value >> shift) | (value << (64 - shift));
#endif
}

_RETURN_ uint32_t kr::math::pow2ceil(uint32_t number) noexcept
{
#ifdef _MSC_VER
	if (number <= 1)
		return 1;
	uint32_t out;
	static_assert(sizeof(unsigned long) == sizeof(uint32_t), "long size unmatch");
	_BitScanReverse((unsigned long*)&out, number - 1);
	return 1 << (out + 1);
#else
	if (number <= 1)
		return 1;
	number--;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	number++;
	return number;
#endif
}
_RETURN_ uint32_t kr::math::pow2floor(uint32_t number) noexcept
{
#ifdef _MSC_VER
	if (number <= 1)
		return 1;
	uint32_t out;
	static_assert(sizeof(unsigned long) == sizeof(uint32_t), "long size unmatch");
	_BitScanReverse((unsigned long*)&out, number);
	return 1 << (out);
#else
	number >>= 1;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	number++;
	return number;
#endif
}
template <> float kr::math::pow<float>(float base, float exp) noexcept
{
	return ::powf(base, exp);
}
template <> double kr::math::pow<double>(double base, double exp) noexcept
{
	return ::pow(base, exp);
}
template <> float kr::math::abs<float>(float a) noexcept
{
	return ::fabs(a);
}
template <> double kr::math::abs<double>(double a) noexcept
{
	return ::fabs(a);
}

const float XM_PI = 3.141592654f;
const float XM_2PI = 6.283185307f;
const float XM_1DIV2PI = 0.159154943f;
const float XM_PIDIV2 = 1.570796327f;

_RETURN_ float kr::math::sin(float a) noexcept
{
	// Map a to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = XM_1DIV2PI*a;
	if (a >= 0.0f)
	{
		quotient = (float)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = a - XM_2PI*quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(a).
	if (y > XM_PIDIV2)
	{
		y = XM_PI - y;
	}
	else if (y < -XM_PIDIV2)
	{
		y = -XM_PI - y;
	}

	// 7-degree minimax approximation
	float y2 = y * y;
	return (((-0.00018524670f * y2 + 0.0083139502f) * y2 - 0.16665852f) * y2 + 1.0f) * y;
	//return ::sin(a);
}
_RETURN_ float kr::math::cos(float a) noexcept
{
	// Map a to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = XM_1DIV2PI*a;
	if (a >= 0.0f)
	{
		quotient = (float)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = a - XM_2PI*quotient;

	// Map y to [-pi/2,pi/2] with cos(y) = sign*cos(x).
	float sign;
	if (y > XM_PIDIV2)
	{
		y = XM_PI - y;
		sign = -1.0f;
	}
	else if (y < -XM_PIDIV2)
	{
		y = -XM_PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = +1.0f;
	}

	// 6-degree minimax approximation
	float y2 = y * y;
	float p = ((-0.0012712436f * y2 + 0.041493919f) * y2 - 0.49992746f) * y2 + 1.0f;
	return sign*p;
	//return ::cos(a);
}
kr::SinCos kr::math::sincos(float a) noexcept
{
	// Map a to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = XM_1DIV2PI*a;
	if (a >= 0.0f)
	{
		quotient = (float)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = a - XM_2PI*quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(a).
	float sign;
	if (y > XM_PIDIV2)
	{
		y = pi - y;
		sign = -1.0f;
	}
	else if (y < -XM_PIDIV2)
	{
		y = -pi - y;
		sign = -1.0f;
	}
	else
	{
		sign = +1.0f;
	}

	float y2 = y * y;

	SinCos out;
	// 7-degree minimax approximation
	out.sin = (((-0.00018524670f * y2 + 0.0083139502f) * y2 - 0.16665852f) * y2 + 1.0f) * y;

	// 6-degree minimax approximation
	float p = ((-0.0012712436f * y2 + 0.041493919f) * y2 - 0.49992746f) * y2 + 1.0f;
	out.cos = sign*p;
	return out;
}
_RETURN_ float kr::math::tan(float a) noexcept
{
	return ::tan(a);
}
_RETURN_ float kr::math::asin(float a) noexcept
{
	return ::asin(a);
}
_RETURN_ float kr::math::acos(float a) noexcept
{
	return ::acos(a);
}
_RETURN_ float kr::math::atan(float a) noexcept
{
	return ::atan(a);
}
_RETURN_ float kr::math::atan2(float y, float x) noexcept
{
	return std::atan2(y, x);
}
_RETURN_ uint32_t kr::math::gap(uint32_t a, uint32_t b) noexcept
{
	return abs((int32_t)((a)-(b)));
}
_RETURN_ float kr::math::ceil(float a) noexcept
{
	return std::ceilf(a);
}
_RETURN_ uint32_t kr::math::ceil(uint32_t a, uint32_t b) noexcept
{
	return (a+b-1)/b*b;
}
_RETURN_ float kr::math::ceil(float a, float b) noexcept
{
	return std::ceilf(a/b)*b;
}
_RETURN_ float kr::math::round(float x) noexcept
{
	return std::roundf(x);
}
_RETURN_ uint32_t kr::math::lround(float x) noexcept
{
	return (uint32_t)std::lroundf(x);
}
template <> float kr::math::tround<float>(float x) noexcept
{
	return std::roundf(x);
}
template <> double kr::math::tround<double>(float x) noexcept
{
	return std::roundf(x);
}
template <> unsigned long long kr::math::tround<unsigned long long>(float x) noexcept
{
	return std::llroundf(x);
}
template <> long long kr::math::tround<long long>(float x) noexcept
{
	return std::llroundf(x);
}
_RETURN_ float kr::math::floor(float a) noexcept
{
	return std::floorf(a);
}
template <>
_RETURN_ float kr::math::mod(float a, float b) noexcept
{
	return std::fmodf(a, b);
}
template <>
_RETURN_ double kr::math::mod(double a, double b) noexcept
{
	return std::fmod(a, b);
}
_RETURN_ float kr::math::radianmod(float a) noexcept
{
	return std::fmodf(a + pi * 3, tau) - pi;
}
_RETURN_ float kr::math::uradianmod(float a) noexcept
{
	return std::fmodf(a + tau, tau);
}
_RETURN_ double kr::math::radianmod(double a) noexcept
{
	return std::fmod(a + pid * 3, taud) - pid;
}
_RETURN_ double kr::math::uradianmod(double a) noexcept
{
	return std::fmod(a + taud, taud);
}
_RETURN_ uint32_t kr::math::ilog(uint32_t val, uint32_t n) noexcept
{
	uint32_t t = 0;
	do
	{
		t++;
	}
	while ((val / n) != 0);
	return t;
}
_RETURN_ uint32_t kr::math::ilog2(uint32_t val) noexcept
{
#if defined(_MSC_VER)
	#ifdef _M_IX86
	#  pragma warning(disable : 4035)
		__asm
		{
			xor      eax, eax
			dec      eax
			bsr      eax, val
		}
	#  pragma warning(default : 4035)
	#else
	uint32_t out;
	static_assert(sizeof(unsigned long) == sizeof(uint32_t), "unsigned long size unmatch");
	return out | _BitScanReverse((unsigned long*)&out, val) - 1;
	#endif
#elif defined(__GNUC__) && (defined(__i386__) || defined(i386))
	int a;
	__asm__ ("\
		 xorl  %0, %0;\
		 decl  %0       ;\
		 bsrl  %1, %0      ;\
		 "
		 : "=&r" (a)
		 : "mr" (val)
		 : "cc"
		 );
	return a;
#elif defined(__GNUC__) && defined(__powerpc__)
	int a;
	__asm__("cntlzw %0,%1"
		: "=r" (a)
		: "r" (val)
		);
	return 31 - a;
#else
	int i;
	for (i = -1; val; ++i, val >>= 1);
	return (i);
#endif
}
_RETURN_ uint32_t kr::math::plog2(uintptr_t val) noexcept
{
#if defined(_MSC_VER)
#ifdef _M_IX86
#  pragma warning(disable : 4035)
	__asm
	{
		xor      eax, eax
		dec      eax
		bsr      eax, val
	}
#  pragma warning(default : 4035)
#else
	unsigned long out;
	static_assert(sizeof(unsigned long) == sizeof(uint32_t), "unsigned long size unmatch");
	return out | _BitScanReverse64(&out, val) - 1;
#endif
#elif defined(__GNUC__) && (defined(__i386__) || defined(i386))
	int a;
	__asm__("\
		 xorl  %0, %0;\
		 decl  %0       ;\
		 bsrl  %1, %0      ;\
		 "
		: "=&r" (a)
		: "mr" (val)
		: "cc"
		);
	return a;
#elif defined(__GNUC__) && defined(__powerpc__)
	uint32_t a;
	__asm__("cntlzw %0,%1"
		: "=r" (a)
		: "r" (val)
		);
	return 31 - a;
#else
	uint32_t i;
	for (i = -1; val; ++i, val >>= 1);
	return (i);
#endif
}


#pragma optimize("",on)