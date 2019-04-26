#pragma once

namespace kr
{
	namespace _pri_
	{
		constexpr kr::dword makeSignature(const char * str, size_t size)
		{
			return size == 0 ? 0 : ((makeSignature(str+1,size-1) << 8) | *str);
		}
	}

	inline dword& hidword(qword & qw)
	{
		return *((dword*)&qw + 1);
	}
	inline dword& lodword(qword & qw)
	{
		return *(dword*)&qw;
	}
	inline word& hiword(dword & qw)
	{
		return *((word*)&qw + 1);
	}
	inline word& loword(dword & qw)
	{
		return *(word*)&qw;
	}
	inline byte& hibyte(word & qw)
	{
		return *((byte*)&qw + 1);
	}
	inline byte& lobyte(word & qw)
	{
		return *(byte*)&qw;
	}

	namespace _pri_
	{
		template <typename T> struct __PDECLTYPE2
		{
			typedef T type;
		};

		template <typename FIRST, typename ... ARGS> struct __GET_FIRST
		{
			typedef FIRST first;
		};

		template <typename T> __PDECLTYPE2<T> __PDECLTYPE(T* p);
	}

	#define pdecltype(x)	decltype(::kr::_pri_::__PDECLTYPE(x))::type

	template <typename T2> static void callDestructor(T2* t2)
	{
		t2->~T2();
	}

	namespace _pri_
	{
		inline void _unpack(...) {};
	}

	constexpr kr::dword operator ""_sig(const char * str, size_t size)
	{
		return kr::_pri_::makeSignature(str, size);
	}

	enum class Charset
	{
		Utf8,
		EucKr,
#ifdef EMSCRIPTEN
		Default = Utf8,
#else
		Default = EucKr,
#endif
	};

	namespace _pri_
	{
		template <bool b, typename ... args>
		struct StaticAssertForType
		{
			static_assert(b, "error");
			static constexpr bool value = true;
		};
	}
}
#define static_assert_with_type(cmp, ...) static_assert(::kr::_pri_::StaticAssertForType<cmp, __VA_ARGS__>::value, #cmp);

#define unpackR(...)  { ::kr::_pri_::_unpack(((__VA_ARGS__), 0) ...); }
#define unpack(...)  { using expander = int[]; (void)expander {0, ((__VA_ARGS__), 0)...}; }

#define CHARSET_CONSTLIZE(charset, code) \
switch (charset) { \
case ::kr::Charset::EucKr: {constexpr ::kr::Charset charset = ::kr::Charset::EucKr; code; } break; \
case ::kr::Charset::Utf8: {constexpr ::kr::Charset charset = ::kr::Charset::Utf8; code; } break; \
}