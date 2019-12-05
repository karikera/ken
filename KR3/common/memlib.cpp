#include "stdafx.h"
#include "memlib.h"

#ifdef _MSC_VER

#pragma intrinsic(memset)
#pragma intrinsic(memcpy)

#pragma optimize("ts",on)

#endif

template struct kr::memt<1>;
template struct kr::memt<2>;
template struct kr::memt<4>;

#define _VOID_				ATTR_NOALIAS void
#define _RETURN_			ATTR_NO_DISCARD ATTR_NOALIAS
#define _NULLABLE_			ATTR_NULLABLE _RETURN_
#define _NOTNULL_			ATTR_NONULL _RETURN_

_NULLABLE_ kr::ptr kr::memtri::find(cptr _src, cptr _tar) noexcept
{
	byte* src = (byte*)_src;
	byte* tar = (byte*)_tar;
	for (;;)
	{
		if (*(word*)src == *(word*)tar)
		{
			if (src[2] == tar[2]) return src;
		}
		src += 3;
		tar += 3;
	}
}
_NULLABLE_ kr::ptr kr::memtri::find(cptr _src, cptr _tar, size_t _srclen) noexcept
{
	byte* src = (byte*)_src;
	byte* end = src + _srclen * 3;
	byte* tar = (byte*)_tar;
	while (src != end)
	{
		if (*(word*)src == *(word*)tar)
		{
			if (src[2] == tar[2]) return src;
		}
		src += 3;
		tar += 3;
	}
	return nullptr;
}
_RETURN_ size_t kr::memtri::pos(cptr _src, cptr _tar) noexcept
{
	return ((byte*)find(_src, _tar) - (byte*)_src)/3;
}
_RETURN_ size_t kr::memtri::pos(cptr _src, cptr _tar, size_t _srclen) noexcept
{
	byte* str=(byte*)find(_src, _tar, _srclen);
	if(str==nullptr) return -1;
	return (str-(byte*)_src)/3;
}

_VOID_ kr::_pri_::SIZE_MEM_SINGLE<1>::zero(void* dest) noexcept
{
	*(dword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<2>::zero(void* dest) noexcept
{
	*(word*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<3>::zero(void* dest) noexcept
{
	*((word*&)dest)++ = 0;
	*(dword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<4>::zero(void* dest) noexcept
{
	*(dword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<5>::zero(void* dest) noexcept
{
	*((dword*&)dest)++ = 0;
	*(dword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<6>::zero(void* dest) noexcept
{
	*((dword*&)dest)++ = 0;
	*(word*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<8>::zero(void* dest) noexcept
{
	*(qword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<9>::zero(void* dest) noexcept
{
	*((qword*&)dest)++ = 0;
	*(dword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<10>::zero(void* dest) noexcept
{
	*((qword*&)dest)++ = 0;
	*(word*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<12>::zero(void* dest) noexcept
{
	*((qword*&)dest)++ = 0;
	*(dword*)dest = 0;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<1>::copy(void* dest, const void* src) noexcept
{
	*(dword*)dest = *(const dword*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<2>::copy(void* dest, const void* src) noexcept
{
	*(word*)dest = *(const word*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<3>::copy(void* dest, const void* src) noexcept
{
	*((word*&)dest)++ = *((const word*&)src)++;
	*(dword*)dest = *(const dword*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<4>::copy(void* dest, const void* src) noexcept
{
	*(dword*)dest = *(const dword*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<5>::copy(void* dest, const void* src) noexcept
{
	*((dword*&)dest)++ = *((const dword*&)src)++;
	*(dword*)dest = *(const dword*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<6>::copy(void* dest, const void* src) noexcept
{
	*((dword*&)dest)++ = *((const dword*&)src)++;
	*(word*)dest = *(const word*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<8>::copy(void* dest, const void* src) noexcept
{
	*(qword*)dest = *(const qword*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<9>::copy(void* dest, const void* src) noexcept
{
	*((qword*&)dest)++ = *((const qword*&)src)++;
	*(dword*)dest = *(const dword*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<10>::copy(void* dest, const void* src) noexcept
{
	*((qword*&)dest)++ = *((const qword*&)src)++;
	*(word*)dest = *(const word*)src;
}
_VOID_ kr::_pri_::SIZE_MEM_SINGLE<12>::copy(void* dest, const void* src) noexcept
{
	*((qword*&)dest)++ = *((const qword*&)src)++;
	*(dword*)dest = *(const dword*)src;
}

#pragma optimize("",on)
