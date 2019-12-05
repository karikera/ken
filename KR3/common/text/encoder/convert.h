#pragma once

#include <KR3/main.h>
#include <KR3/meta/if.h>

#include "encoder.h"

namespace kr
{
	template <typename ToConverter, typename FromConverter>
	struct TransConverter
		: encoder::Encoder<TransConverter<ToConverter, FromConverter>, typename ToConverter::ToComponent, typename FromConverter::FromComponent>
	{
		static_assert(is_same<typename ToConverter::FromComponent, typename FromConverter::ToComponent>::value, "Converter type missmatch");

		using Super = encoder::Encoder<TransConverter<ToConverter, FromConverter>, typename ToConverter::ToComponent, typename FromConverter::FromComponent>;
	public:
		using Super::Super;
		using From = typename Super::FromComponent;
		using M = typename ToConverter::FromComponent;
		using To = typename Super::ToComponent;
		using typename Super::Decoder;
		using typename Super::FromText;
		using typename Super::ToText;
		using MText = View<M>;
		using MWriter = ArrayWriter<M>;

		static size_t length(FromText text) noexcept;
		static size_t encode(To * out, FromText text) noexcept;
		static size_t delength(ToText text) noexcept;
		static size_t decode(From * out, ToText text) noexcept;
	};

	class Utf16ToUtf32
		: public encoder::Encoder<Utf16ToUtf32, char32, char16>
	{
		using Super = encoder::Encoder<Utf16ToUtf32, char32, char16>;
	public:
		using Super::Super;

		static size_t length(Text16 text) noexcept;
		static size_t encode(char32 * out, Text16 text) noexcept;
		static size_t delength(Text32 text) noexcept;
		static size_t decode(char16 * out, Text32 text) noexcept;
	};;

	template <Charset charset, typename To> class ToConvert
		: public encoder::Encoder<ToConvert<charset, To>, To, char>
	{
		using Super = encoder::Encoder<ToConvert<charset, To>, To, char>;
	public:
		using Super::Super;
		using typename Super::Decoder;
		using typename Super::ToText;

		static size_t length(Text text) noexcept;
		static size_t encode(To * out, Text text) noexcept;
		static size_t delength(ToText text) noexcept;
		static size_t decode(char * out, ToText text) noexcept;
	};;

	template <Charset charset>
	class meml
	{
	public:
		static bool isDbcs(char chr) noexcept;
		static char* next(const char* str) noexcept;
		static char * find(const char * _src, char _tar) noexcept;
		static char * find(const char * _src, char _tar, size_t _srclen) noexcept;
		static char * find_e(const char * _src, char _tar, size_t _srclen) noexcept;
		static char * find_n(const char * _src, char _tar) noexcept;
		static char * find_n(const char * _src, char _tar, size_t _srclen) noexcept;
		static char * find_ne(const char * _src, char _tar, size_t _srclen) noexcept;
		static Text next(Text str) noexcept;
		static Text find(Text txt, char tar) noexcept;
		static Text find_e(Text txt, char tar) noexcept;
		static Text find_n(Text txt, char tar) noexcept;
		static Text find_ne(Text txt, char tar) noexcept;
	};

	template <Charset charset> class ToConvert<charset, char>;

	template <typename ToConverter, typename FromConverter>
	size_t TransConverter<ToConverter, FromConverter>::length(FromText text) noexcept
	{
		TmpArray<M> text_m = FromConverter(text);
		return ToConverter::length(text_m);
	}
	template <typename ToConverter, typename FromConverter>
	size_t TransConverter<ToConverter, FromConverter>::encode(To * out, FromText text) noexcept
	{
		TmpArray<M> text_m = FromConverter(text);
		return ToConverter::encode(out, text_m);
	}
	template <typename ToConverter, typename FromConverter>
	size_t TransConverter<ToConverter, FromConverter>::delength(ToText text) noexcept
	{
		TmpArray<M> text_m = typename ToConverter::Decoder(text);
		return FromConverter::delength(text_m);
	}
	template <typename ToConverter, typename FromConverter>
	size_t TransConverter<ToConverter, FromConverter>::decode(From * out, ToText text) noexcept
	{
		TmpArray<M> text_m = typename ToConverter::Decoder(text);
		return FromConverter::decode(out, text_m);
	}

	template <Charset charset>
	char* meml<charset>::next(const char* str) noexcept
	{
		return (isDbcs(*str) ? 2 : 1) + (char*)str;
	}
	template <Charset charset>
	char * meml<charset>::find(const char * _src, char _tar) noexcept
	{
		for (;;)
		{
			if (*_src == _tar)
				return (char*)_src;
			_src = next(_src);
		}
	}
	template <Charset charset>
	char * meml<charset>::find(const char * _src, char _tar, size_t _srclen) noexcept
	{
		char * srcend = (char*)_src + _srclen;
		for (;;)
		{
			if (*_src == _tar)
				return (char*)_src;
			_src = next(_src);
			if (_src >= srcend)
				return nullptr;
		}
	}
	template <Charset charset>
	char * meml<charset>::find_e(const char * _src, char _tar, size_t _srclen) noexcept
	{
		char * srcend = (char*)_src + _srclen;
		for (;;)
		{
			if (*_src == _tar)
				break;
			_src = next(_src);
			if (_src >= srcend)
				break;
		}
		return (char*)_src;
	}
	template <Charset charset>
	char * meml<charset>::find_n(const char * _src, char _tar) noexcept
	{
		for (;;)
		{
			if (*_src != _tar)
				return (char*)_src;
			_src = next(_src);
		}
	}
	template <Charset charset>
	char * meml<charset>::find_n(const char * _src, char _tar, size_t _srclen) noexcept
	{
		char * srcend = (char*)_src + _srclen;
		for (;;)
		{
			if (*_src != _tar)
				return (char*)_src;
			if (isDbcs(*_src++))
			{
				if (_src == srcend)
					return nullptr;
				_src++;
			}
			if (_src == srcend)
				return nullptr;
		}
	}
	template <Charset charset>
	char * meml<charset>::find_ne(const char * _src, char _tar, size_t _srclen) noexcept
	{
		char * srcend = (char*)_src + _srclen;
		for (;;)
		{
			if (*_src != _tar)
				break;
			if (isDbcs(*_src++))
			{
				if (_src == srcend)
					break;
				_src++;
			}
			if (_src == srcend)
				break;
		}
		return (char*)_src;
	}
	template <Charset charset>
	Text meml<charset>::next(Text str) noexcept
	{
		_assert(!str.empty());
		if (!isDbcs(*str++))
			return str;
		if (str.empty())
			return str;
		str++;
		return str;
	}
	template <Charset charset>
	Text meml<charset>::find(Text txt, char tar) noexcept
	{
		const char * finded = find(txt.begin(), tar, txt.size());
		txt.setBegin(finded);
		return txt;
	}
	template <Charset charset>
	Text meml<charset>::find_e(Text txt, char tar) noexcept
	{
		const char * finded = find_e(txt.begin(), tar, txt.size());
		txt.setBegin(finded);
		return txt;
	}
	template <Charset charset>
	Text meml<charset>::find_n(Text txt, char tar) noexcept
	{
		const char * finded = find_n(txt.begin(), tar, txt.size());
		txt.setBegin(finded);
		return txt;
	}
	template <Charset charset>
	Text meml<charset>::find_ne(Text txt, char tar) noexcept
	{
		const char * finded = find_ne(txt.begin(), tar, txt.size());
		txt.setBegin(finded);
		return txt;
	}

	namespace _pri_
	{
		template <typename To, typename From>
		struct UnicodeConverterImpl
		{
			static_assert(is_same<To, From>::value, "Unknown encoding time");
			using type = encoder::PassEncoder<To>;
		};

		template <typename To>
		struct UnicodeConverterImpl<To, char>
		{
			using type = ToConvert<Charset::Utf8, To>;
		};
		template <typename From>
		struct UnicodeConverterImpl<char, From>
		{
			using type = typename ToConvert<Charset::Utf8, From>::Decoder;
		};
		template <>
		struct UnicodeConverterImpl<char32, char16>
		{
			using type = Utf16ToUtf32;
		};
		template <>
		struct UnicodeConverterImpl<char16, char32>
		{
			using type = Utf16ToUtf32::Decoder;
		};
	}
	template <typename To, typename From>
	using UnicodeConverter = typename _pri_::UnicodeConverterImpl<To, From>::type;

	using Utf32ToUtf16 = Utf16ToUtf32::Decoder;

	template <Charset charset>
	using MultiByteToUtf32 = ToConvert<charset, char32>;
	template <Charset charset>
	using MultiByteToUtf16 = ToConvert<charset, char16>;
	template <Charset charset>
	using Utf32ToMultiByte = typename MultiByteToUtf32<charset>::Decoder;
	template <Charset charset>
	using Utf16ToMultiByte = typename MultiByteToUtf16<charset>::Decoder;

	template <typename T>
	using FromUtf8 = ToConvert<Charset::Utf8, T>;
	template <typename T>
	using ToUtf8 = typename FromUtf8<T>::Decoder;
	
	using Utf16ToUtf8 = Utf16ToMultiByte<Charset::Utf8>;
	using Utf8ToUtf16 = MultiByteToUtf16<Charset::Utf8>;
	using Utf32ToUtf8 = Utf32ToMultiByte<Charset::Utf8>;
	using Utf8ToUtf32 = MultiByteToUtf32<Charset::Utf8>;

	template <typename T>
	using FromNone = ToConvert<Charset::None, T>;
	template <typename T>
	using ToNone = typename FromNone<T>::Decoder;

	using Utf16ToNone = Utf16ToMultiByte<Charset::None>;
	using NoneToUtf16 = MultiByteToUtf16<Charset::None>;
	using Utf32ToNone = Utf32ToMultiByte<Charset::None>;
	using NoneToUtf32 = MultiByteToUtf32<Charset::None>;

	template <typename T>
	using FromAcp = ToConvert<Charset::Default, T>;
	template <typename T>
	using ToAcp = typename FromAcp<T>::Decoder;

	using Utf16ToAnsi = Utf16ToMultiByte<Charset::Default>;
	using AnsiToUtf16 = MultiByteToUtf16<Charset::Default>;
	using Utf32ToAnsi = Utf32ToMultiByte<Charset::Default>;
	using AnsiToUtf32 = MultiByteToUtf32<Charset::Default>;

	inline Text16 toUtf16(Text16 tx) noexcept
	{
		return tx;
	}
	inline Utf32ToUtf16 toUtf16(Text32 tx) noexcept
	{
		return tx;
	}

	inline Text32 toUtf32(Text32 tx) noexcept
	{
		return tx;
	}
	inline Utf16ToUtf32 toUtf32(Text16 tx) noexcept
	{
		return tx;
	}

	inline NoneToUtf16 noneToUtf16(Text tx) noexcept
	{
		return (NoneToUtf16)tx;
	}
	inline Text16 noneToUtf16(Text16 tx) noexcept
	{
		return tx;
	}
	inline Utf8ToUtf16 utf8ToUtf16(Text tx) noexcept
	{
		return (Utf8ToUtf16)tx;
	}
	inline Text16 utf8ToUtf16(Text16 tx) noexcept
	{
		return tx;
	}
	inline AnsiToUtf16 acpToUtf16(Text tx) noexcept
	{
		return (AnsiToUtf16)tx;
	}
	inline Text16 acpToUtf16(Text16 tx) noexcept
	{
		return tx;
	}

	inline Text toNone(Text tx) noexcept
	{
		return tx;
	}
	inline Utf16ToNone toNone(Text16 tx) noexcept
	{
		return (Utf16ToNone)tx;
	}

	inline Text toUtf8(Text tx) noexcept
	{
		return tx;
	}
	inline Utf16ToUtf8 toUtf8(Text16 tx) noexcept
	{
		return (Utf16ToUtf8)tx;
	}
	
	inline Text toAnsi(Text tx) noexcept
	{
		return tx;
	}
	inline Utf16ToAnsi toAnsi(Text16 tx) noexcept
	{
		return (Utf16ToAnsi)tx;
	}
}

#define EXTERN_CHARSET(charset, C) \
template <> \
size_t kr::ToConvert<charset, C>::length(Text text) noexcept; \
template <> \
size_t kr::ToConvert<charset, C>::encode(C* out, Text text) noexcept; \
template <> \
size_t kr::ToConvert<charset, C>::delength(View<C> text) noexcept; \
template <> \
size_t kr::ToConvert<charset, C>::decode(char* out, View<C> text) noexcept; \

EXTERN_CHARSET(kr::Charset::None, kr::char16);
EXTERN_CHARSET(kr::Charset::None, kr::char32);
EXTERN_CHARSET(kr::Charset::Utf8, kr::char16);
EXTERN_CHARSET(kr::Charset::Utf8, kr::char32);
EXTERN_CHARSET(kr::Charset::Ansi, kr::char16);
EXTERN_CHARSET(kr::Charset::Ansi, kr::char32);

bool kr::meml<kr::Charset::Ansi>::isDbcs(char chr) noexcept;
bool kr::meml<kr::Charset::EucKr>::isDbcs(char chr) noexcept;
bool kr::meml<kr::Charset::Utf8>::isDbcs(char chr) noexcept;

#undef EXTERN_CHARSET

extern template class kr::ToConvert<kr::Charset::Utf8, kr::char16>;
extern template class kr::ToConvert<kr::Charset::Default, kr::char16>;
