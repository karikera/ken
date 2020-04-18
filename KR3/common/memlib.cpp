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

#pragma optimize("",on)
