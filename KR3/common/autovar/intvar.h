#pragma once

namespace kr
{
	// int by size
	namespace _pri_
	{
		template <size_t size> struct int_sz{};
		template <> struct int_sz<1>{ using stype = int8_t;	using utype = uint8_t;	using ctype = char; };
		template <> struct int_sz<2>{ using stype = int16_t;	using utype = uint16_t;	using ctype = char16; };
		template <> struct int_sz<4>{ using stype = int32_t;	using utype = uint32_t;	using ctype = char32; };
		template <> struct int_sz<8>{ using stype = int64_t;	using utype = uint64_t;	using ctype = qword;};
	}

	template <size_t size> using int_sz_t = typename _pri_::int_sz<size>::stype;
	template <size_t size> using uint_sz_t = typename _pri_::int_sz<size>::utype;
	template <size_t size> using char_sz_t = typename _pri_::int_sz<size>::ctype;

	// raw
	template <typename T> int_sz_t<sizeof(T)>& raw(T &data)
	{
		return (int_sz_t<sizeof(T)>&) data;
	}
}
