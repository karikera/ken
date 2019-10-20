#pragma once

#include "../main.h"
#include "../meta/retype.h"

namespace kr
{
	using unicode_wchar_t = char_sz_t<sizeof(wchar_t)>;

	template <typename T>
	inline meta::retype_t<T, wchar_t> wide(T v) noexcept
	{
		using t = meta::retype<T, wchar_t>;
		static_assert(std::is_same<typename t::from, unicode_wchar_t>::value, "wchar type unmatch");
		return (typename t::type)v;
	}
	template <typename T>
	inline meta::retype_t<T, unicode_wchar_t> unwide(T v) noexcept
	{
		using t = meta::retype<T, unicode_wchar_t>;
		static_assert(std::is_same<typename t::from, wchar_t>::value, "wchar type unmatch");
		return (meta::retype_t<T, unicode_wchar_t>)v;
	}
	inline wchar_t* wide(TSZ16& tsz) noexcept
	{
		return wide(tsz.c_str());
	}

	namespace _pri_
	{
		template <typename T>
		struct wide_tmp_type
		{
			using type = TmpArray<wchar_t>;
			static type wide_tmp(const T * str, size_t sz)
			{
				TmpArray<unicode_wchar_t> out;
				out << UnicodeConverter<unicode_wchar_t, T>(View<T>(str, sz));
				return (type&&)(type&)out;
			}
		};
		template <>
		struct wide_tmp_type<unicode_wchar_t>
		{
			using type = View<wchar_t>;
			static type wide_tmp(const unicode_wchar_t * str, size_t sz)
			{
				return type((const wchar_t*)str, sz);
			}
		};
	}

	template <typename T>
	typename _pri_::wide_tmp_type<T>::type wide_tmp(const T * str, size_t sz)
	{
		return _pri_::wide_tmp_type<T>::wide_tmp(str, sz);
	}

	const wchar_t * szlize(Text16 text, TText16* buffer) noexcept;
}
