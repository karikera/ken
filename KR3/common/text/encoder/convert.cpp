#include "stdafx.h"
#include "convert.h"

#include <cstring>
#include <cwchar>
#include <assert.h>


using namespace kr;

constexpr uint SURROGATE_MASK = 0xfc00;
constexpr uint SURROGATE_HIGH = 0xd800;
constexpr uint SURROGATE_LOW = 0xdc00;
constexpr uint SURROGATE_VALUE_MASK = 0x04c0;
constexpr uint SURROGATE_VALUE_DIFF = 0x0040;

constexpr char16 REPLACEMENT_CHARACTER = 0xfffd; // 표기 불가

namespace
{
	uint surrogateToUnicode(char16 high, char16 low) noexcept
	{
		return ((high << 10) + low) - (((SURROGATE_HIGH - SURROGATE_VALUE_DIFF) << 10) + SURROGATE_LOW);
	}
	struct SurrogatePair
	{
		char16 high;
		char16 low;
	};
	SurrogatePair unicodeToSurrogate(uint unicode) noexcept
	{
		SurrogatePair pair;
		pair.high = (char16)(((unicode >> 10) | SURROGATE_HIGH) - SURROGATE_VALUE_DIFF);
		pair.low = (char16)((unicode & 0x03ff) | SURROGATE_LOW);
		return pair;
	}
}

size_t Utf16ToUtf32::length(Text16 text) noexcept
{
	size_t sz = 0;
	while (!text.empty())
	{
		char16 chr = *text++;
		if ((word)chr <= 0x7ff)
			sz++;
		else if ((chr & SURROGATE_MASK) == SURROGATE_HIGH)
		{
			if (text.empty())
				break;
			text++;
			sz++;
		}
		else
			sz++;
	}
	return sz;
}
size_t Utf16ToUtf32::encode(char32 * out, Text16 text) noexcept
{
	char32 * dest = out;
	while (!text.empty())
	{
		char16 chr = text.front();
		if ((word)chr <= 0x7ff)
		{
			*dest++ = chr;
		}
		else if ((chr & 0xfc00) == 0xd800)
		{
			if (text.empty())
				break;
			text++;
			*dest ++ = (char32)surrogateToUnicode(chr, text.front());
		}
		else
		{
			*dest++ = chr;
		}
		text++;
	}
	return dest - out;
}
size_t Utf16ToUtf32::delength(Text32 text) noexcept
{
	size_t sz = 0;
	for (char32 chr : text)
	{
		if ((dword)chr <= 0xffff)
			sz++;
		else if ((dword)chr <= 0x1fffff)
			sz += 2;
		else
			sz++;
	}
	return sz;
}
size_t Utf16ToUtf32::decode(char16 * out, Text32 text) noexcept
{
	char16 * dest = out;

	while (!text.empty())
	{
		char32 chr = text.front();
		if ((dword)chr <= 0xffff)
		{
			*dest++ = (char16)chr;
		}
		else if ((dword)chr <= 0x1fffff)
		{
			SurrogatePair pair = unicodeToSurrogate(*text);
			*dest++ = pair.high;
			*dest++ = pair.low;
		}
		else
		{
			*dest++ = REPLACEMENT_CHARACTER;
		}
		text++;
	}
	return dest - out;
}

template <>
size_t ToConvert<Charset::None, char16>::length(Text text) noexcept
{
	return text.size();
}
template <>
size_t ToConvert<Charset::None, char16>::encode(char16* out, Text text) noexcept
{
	for (char chr : text)
	{
		*out++ = chr;
	}
	return text.size();
}
template <>
size_t ToConvert<Charset::None, char16>::delength(Text16 text) noexcept
{
	return text.size();
}
template <>
size_t ToConvert<Charset::None, char16>::decode(char* out, Text16 text) noexcept
{
	for (char16 chr : text)
	{
		*out++ = (char)chr;
	}
	return text.size();
}

template <>
size_t ToConvert<Charset::None, char32>::length(Text text) noexcept
{
	return text.size();
}
template <>
size_t ToConvert<Charset::None, char32>::encode(char32* out, Text text) noexcept
{
	for (char chr : text)
	{
		*out++ = chr;
	}
	return text.size();
}
template <>
size_t ToConvert<Charset::None, char32>::delength(Text32 text) noexcept
{
	return text.size();
}
template <>
size_t ToConvert<Charset::None, char32>::decode(char* out, Text32 text) noexcept
{
	for (char32 chr : text)
	{
		*out++ = (char)chr;
	}
	return text.size();
}


template <>
size_t ToConvert<Charset::Utf8, char16>::length(Text text) noexcept
{
	size_t sz = 0;
	while (!text.empty())
	{
		char chr = text.readForce();
		if ((chr & 0x80) == 0)
		{
			sz++;
		}
		else if ((chr & 0xe0) == 0xc0)
		{
			if (text.size() < 1)
				break;
			text++;
			sz++;
		}
		else if ((chr & 0xf0) == 0xe0)
		{
			if (text.size() < 2)
				break;
			text += 2;
			sz++;
		}
		else if ((chr & 0xf8) == 0xf0)
		{
			if (text.size() < 3)
				break;
			char second = text.readForce();
			text += 2;

			uint surrogate = (byte)(chr & 0x7) << 6;
			surrogate |= (byte)(second & 0x30);
			if (surrogate > (0x10 << 4))
				sz++;
			else
				sz += 2;
		}
		else if ((chr & 0xfc) == 0xf8)
		{
			if (text.size() < 4)
				break;
			text += 4;
			sz++;
		}
		else if ((chr & 0xfe) == 0xfc)
		{
			if (text.size() < 5)
				break;
			text += 5;
			sz++;
		}
		else
			sz++;
	}
	return sz;
}
template <>
size_t ToConvert<Charset::Utf8, char16>::encode(char16 * out, Text text) noexcept
{
	char16 * dest = out;

	while (!text.empty())
	{
		char chr = text.front();
		if ((chr & 0x80) == 0)
		{
			*dest++ = chr;
			text++;
		}
		else if ((chr & 0xe0) == 0xc0)
		{
			if (text.size() < 2)
				break;
			text++;
			char16 res = (byte)(chr & 0x1f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else if ((chr & 0xf0) == 0xe0)
		{
			if (text.size() < 3)
				break;
			text++;
			char16 res = (byte)(chr & 0x0f) << 12;
			res |= (byte)(text.readForce() & 0x3f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else if ((chr & 0xf8) == 0xf0)
		{
			if (text.size() < 4)
				break;
			text++;
			char second = text.readForce();
			uint surrogate = (byte)(chr & 0x7) << 6;
			surrogate |= (byte)(second & 0x30);
			if (surrogate > (0x10 << 4))
			{
				text += 2;
				*dest++ = REPLACEMENT_CHARACTER;
			}
			else
			{
				char third = text.readForce();

				surrogate -= (1 << 4);

				char16 res = (char16)(surrogate << 2);
				res |= (byte)(second & 0xf) << 2;
				res |= (byte)(third & 0x3f) >> 4;
				*dest++ = (res | 0xd800);

				res = (byte)(third & 0xf) << 6;
				res |= (byte)(text.readForce() & 0x3f);
				*dest++ = (res | 0xdc00);
			}
		}
		else if ((chr & 0xfc) == 0xf8)
		{
			if (text.size() < 5)
				break;
			text+=6;
			*dest++ = REPLACEMENT_CHARACTER;
		}
		else if ((chr & 0xfe) == 0xfc)
		{
			if (text.size() < 6)
				break;
			text += 6;
			*dest++ = REPLACEMENT_CHARACTER;
		}
		else
		{
			text++;
			*dest++ = REPLACEMENT_CHARACTER;
		}
	}
	return dest - out;
}
template <>
size_t ToConvert<Charset::Utf8, char16>::delength(Text16 text) noexcept
{
	size_t sz = 0;
	while (!text.empty())
	{
		char16 chr = *text++;
		if ((word)chr <= 0x7f)
			sz++;
		else if ((word)chr <= 0x7ff)
			sz += 2;
		else if ((chr & SURROGATE_MASK) == SURROGATE_HIGH)
		{
			if (text.empty())
				break;
			text++;
			sz += 4;
		}
		else
			sz += 3;
	}
	return sz;
}
template <>
size_t ToConvert<Charset::Utf8, char16>::decode(char * out, Text16 text) noexcept
{
	char * dest = out;
	while (!text.empty())
	{
		char16 chr = text.front();
		if ((word)chr <= 0x7f)
		{
			*dest++ = (char)chr;
		}
		else if ((word)chr <= 0x7ff)
		{
			*dest++ = (char)((chr >> 6) | 0xc0);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		else if ((chr & SURROGATE_MASK) == SURROGATE_HIGH)
		{
			if (text.empty())
				break;
			text++;
			uint unicode = surrogateToUnicode(chr, text.front());
			*dest++ = (char)((unicode >> 18) | 0xf0);
			*dest++ = (char)(((unicode >> 12) & 0x3f) | 0x80);
			*dest++ = (char)(((unicode >> 6) & 0x3f) | 0x80);
			*dest++ = (char)((unicode & 0x3f) | 0x80);
		}
		else
		{
			*dest++ = (char)((chr >> 12) | 0xe0);
			*dest++ = (char)(((chr >> 6) & 0x3f) | 0x80);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		text++;
	}
	return dest - out;
}

template <>
size_t ToConvert<Charset::Utf8, char32>::length(Text text) noexcept
{
	size_t sz = 0;
	while (!text.empty())
	{
		char chr = text.readForce();
		if ((chr & 0x80) == 0)
		{
			sz++;
		}
		else if ((chr & 0xe0) == 0xc0)
		{
			if (text.size() < 1)
				break;
			text++;
			sz++;
		}
		else if ((chr & 0xf0) == 0xe0)
		{
			if (text.size() < 2)
				break;
			text += 2;
			sz++;
		}
		else if ((chr & 0xf8) == 0xf0)
		{
			if (text.size() < 3)
				break;
			text += 3;
			sz++;
		}
		else if ((chr & 0xfc) == 0xf8)
		{
			if (text.size() < 4)
				break;
			text += 4;
			sz++;
		}
		else if ((chr & 0xfe) == 0xfc)
		{
			if (text.size() < 5)
				break;
			text += 5;
			sz++;
		}
	}
	return sz;
}
template <>
size_t ToConvert<Charset::Utf8, char32>::encode(char32 * out, Text text) noexcept
{
	char32 * dest = out;

	while (!text.empty())
	{
		char chr = text.front();
		if ((chr & 0x80) == 0)
		{
			*dest++ = chr;
			text++;
		}
		else if ((chr & 0xe0) == 0xc0)
		{
			if (text.size() < 2)
				break;
			text++;
			char32 res = (byte)(chr & 0x1f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else if ((chr & 0xf0) == 0xe0)
		{
			if (text.size() < 3)
				break;
			text++;
			char32 res = (byte)(chr & 0x0f) << 12;
			res |= (byte)(text.readForce() & 0x3f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else if ((chr & 0xf8) == 0xf0)
		{
			if (text.size() < 4)
				break;
			text++;
			char32 res = (byte)(chr & 0x07) << 18;
			res |= (byte)(text.readForce() & 0x3f) << 12;
			res |= (byte)(text.readForce() & 0x3f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else if ((chr & 0xfc) == 0xf8)
		{
			if (text.size() < 5)
				break;
			text++;
			char32 res = (byte)(chr & 0x03) << 24;
			res |= (byte)(text.readForce() & 0x3f) << 18;
			res |= (byte)(text.readForce() & 0x3f) << 12;
			res |= (byte)(text.readForce() & 0x3f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else if ((chr & 0xfe) == 0xfc)
		{
			if (text.size() < 6)
				break;
			text++;
			char32 res = (byte)(chr & 0x01) << 30;
			res |= (byte)(text.readForce() & 0x3f) << 24;
			res |= (byte)(text.readForce() & 0x3f) << 18;
			res |= (byte)(text.readForce() & 0x3f) << 12;
			res |= (byte)(text.readForce() & 0x3f) << 6;
			res |= (byte)(text.readForce() & 0x3f);
			*dest++ = res;
		}
		else
		{
			text++;
			*dest++ = 0xfffd;
		}
	}
	return dest - out;
}
template <>
size_t ToConvert<Charset::Utf8, char32>::delength(Text32 text) noexcept
{
	size_t sz = 0;
	for (char32 chr : text)
	{
		if ((dword)chr <= 0x7f)
			sz++;
		else if ((dword)chr <= 0x7ff)
			sz += 2;
		else if ((dword)chr <= 0x7fff)
			sz += 3;
		else if ((dword)chr <= 0x7ffff)
			sz += 4;
		else if ((dword)chr <= 0x7fffff)
			sz += 5;
		else
			sz += 6;
	}
	return sz;
}
template <>
size_t ToConvert<Charset::Utf8, char32>::decode(char * out, Text32 text) noexcept
{
	char * dest = out;
	while (!text.empty())
	{
		char32 chr = text.front();
		if ((dword)chr <= 0x7f)
		{
			*dest++ = (char)chr;
		}
		else if ((dword)chr <= 0x7ff)
		{
			*dest++ = (char)((chr >> 6) | 0xc0);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		else if ((dword)chr <= 0x7fff)
		{
			*dest++ = (char)((chr >> 12) | 0xe0);
			*dest++ = (char)(((chr >> 6) & 0x3f) | 0x80);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		else if ((dword)chr <= 0x7ffff)
		{
			*dest++ = (char)((chr >> 18) | 0xf0);
			*dest++ = (char)(((chr >> 12) & 0x3f) | 0x80);
			*dest++ = (char)(((chr >> 6) & 0x3f) | 0x80);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		else if ((dword)chr <= 0x7fffff)
		{
			*dest++ = (char)((chr >> 24) | 0xf8);
			*dest++ = (char)(((chr >> 18) & 0x3f) | 0x80);
			*dest++ = (char)(((chr >> 12) & 0x3f) | 0x80);
			*dest++ = (char)(((chr >> 6) & 0x3f) | 0x80);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		else
		{
			*dest++ = (char)((chr >> 30) | 0xfc);
			*dest++ = (char)(((chr >> 24) & 0x3f) | 0x80);
			*dest++ = (char)(((chr >> 18) & 0x3f) | 0x80);
			*dest++ = (char)(((chr >> 12) & 0x3f) | 0x80);
			*dest++ = (char)(((chr >> 6) & 0x3f) | 0x80);
			*dest++ = (char)((chr & 0x3f) | 0x80);
		}
		text++;
	}
	return dest - out;
}


#ifdef WIN32

#include <KR3/wl/windows.h>

namespace
{
	template <int cp>
	struct MsCodePageImpl
	{
		static constexpr int codePage = cp;
		static CPINFO cpinfo;
		static inline void initCpInfo() noexcept
		{
			GetCPInfo(cp, &cpinfo);
		}
	};
	template <int cp>
	CPINFO MsCodePageImpl<cp>::cpinfo;

	template <Charset cs> struct MsCodePage : MsCodePageImpl<0> {};
	template <> struct MsCodePage<Charset::Ansi> : MsCodePageImpl<0> {};
	template <> struct MsCodePage<Charset::EucKr> : MsCodePageImpl<949> {};

	staticCode
	{
		MsCodePage<Charset::Ansi>::initCpInfo();
		MsCodePage<Charset::EucKr>::initCpInfo();
	};

	template <Charset charset, typename To>
	struct MsConvert : encoder::Encoder<MsConvert<charset, To>, To, char>
	{
		static_assert(sizeof(To) == sizeof(WCHAR), "character size not match");
		using Super = encoder::Encoder<MsConvert<charset, To>, To, char>;

		static constexpr int cp = MsCodePage<charset>::codePage;

		using Super::Super;
		using typename Super::ToText;

		static inline size_t length(Text text) noexcept
		{
			if (text.size() == 0)
				return 0;
			return MultiByteToWideChar(cp, 0, text.data(), intact<int>(text.size()), nullptr, 0);
		}
		static inline size_t encode(To * out, Text text)
		{
			if (text.size() == 0)
				return 0;

			int srclen = intact<int>(text.size());
			constexpr int SRCLEN_LIMIT = 3000;
			int dstlen;
			if (srclen >= SRCLEN_LIMIT)
				dstlen = SRCLEN_LIMIT;
			else
				dstlen = srclen;
			int res = MultiByteToWideChar(cp, 0, text.data(), srclen, (LPWSTR)out, dstlen);
			if (res != 0)
				return res;

			int require = MultiByteToWideChar(cp, 0, text.data(), srclen, (LPWSTR)out, 0);
			return MultiByteToWideChar(cp, 0, text.data(), srclen, (LPWSTR)out, require);
		}
		static inline size_t delength(ToText text) noexcept
		{
			if (text.size() == 0)
				return 0;
			return WideCharToMultiByte(cp, 0, (LPCWSTR)text.data(), intact<int>(text.size()), nullptr, 0, nullptr, nullptr);
		}
		static inline size_t decode(char * out, ToText text) noexcept
		{
			if (text.size() == 0)
				return 0;

			int srclen = intact<int>(text.size());
			constexpr int SRCLEN_LIMIT = 3000;
			int dstlen;
			if (srclen >= SRCLEN_LIMIT / 3)
				dstlen = SRCLEN_LIMIT;
			else
				dstlen = srclen * 3;
			int res = WideCharToMultiByte(cp, 0, (LPCWSTR)text.data(), srclen, out, dstlen, nullptr, nullptr);
			if (res != 0)
				return res;

			int require = WideCharToMultiByte(cp, 0, (LPWSTR)text.data(), srclen, out, 0, nullptr, nullptr);
			return WideCharToMultiByte(cp, 0, (LPCWSTR)text.data(), srclen, out, require, nullptr, nullptr);
		}
	};
	template <Charset charset>
	struct MsConvert<charset, char32> :TransConverter<Utf16ToUtf32, MsConvert<charset, char16>>
	{
	};
}

static bool msIsBdcs(char chr, CPINFO* cpinfo) noexcept
{
	uint idx = 0;
	while (idx < cpinfo->MaxCharSize)
	{
		if ((byte)chr < cpinfo->LeadByte[idx])
		{
			idx += 2;
			continue;
		}
		idx++;
		if ((byte)chr > cpinfo->LeadByte[idx])
		{
			idx++;
			continue;
		}
		return true;
	}
	return false;
}


#define EXPORT_CHARSET(charset, C) \
template <> \
size_t kr::ToConvert<charset, C>::delength(View<C> text) noexcept \
{ return MsConvert<charset, C>::delength(text); } \
template <> \
size_t kr::ToConvert<charset, C>::decode(char * out, View<C> text) noexcept \
{ return MsConvert<charset, C>::decode(out, text); } \
template <> \
size_t kr::ToConvert<charset, C>::length(Text text) noexcept \
{ return MsConvert<charset, C>::length(text); } \
template <> \
size_t kr::ToConvert<charset, C>::encode(C * out, Text text) noexcept \
{ return MsConvert<charset, C>::encode(out, text); } \

EXPORT_CHARSET(Charset::Ansi, char16);
EXPORT_CHARSET(Charset::EucKr, char16);
EXPORT_CHARSET(Charset::Ansi, char32);
EXPORT_CHARSET(Charset::EucKr, char32);

template <>
bool meml<Charset::Ansi>::isDbcs(char chr) noexcept
{
	return msIsBdcs(chr, &MsCodePage<Charset::Ansi>::cpinfo);
}
template <>
bool meml<Charset::EucKr>::isDbcs(char chr) noexcept
{
	return msIsBdcs(chr, &MsCodePage<Charset::EucKr>::cpinfo);
}

#else

#endif

template <>
bool meml<Charset::Utf8>::isDbcs(char chr) noexcept
{
	return chr < 0;
}