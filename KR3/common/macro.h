#pragma once

#include "compiler.h"

#include <atomic>
#include <type_traits>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include <initializer_list>
#include <utility>
#include <type_traits>
#include <atomic>
#include <exception>
#include <stdexcept>

namespace kr
{
	using std::initializer_list;
	using std::move;
	using std::forward;
	using std::pair;
}

using std::atomic;
using std::is_base_of;
using std::is_base_of_v;
using std::is_same;
using std::is_same_v;
using std::add_lvalue_reference_t;
using std::add_rvalue_reference_t;
using std::add_pointer_t;
using std::add_const_t;
using std::remove_reference_t;
using std::remove_pointer_t;
using std::remove_const_t;
using std::decay_t;
using std::is_convertible;
using std::is_convertible_v;
template <typename T>
using remove_constptr_t = remove_const_t<remove_pointer_t<T>>;
template <typename T>
using remove_constref_t = remove_const_t<remove_reference_t<T>>;

#define MUST_BASE_OF(der, ...) static_assert((bool)is_base_of<__VA_ARGS__, der>::value,#der " is not base of " #__VA_ARGS__)
#define CLASS_HEADER(this_t,...) \
	using This = this_t;\
	using Super = __VA_ARGS__;

#define EXTERN_FULL_CHAR_CLASS(tclass) \
	extern template class tclass##T<char>;\
	extern template class tclass##T<char16>;\
	extern template class tclass##T<char32>;\
	using tclass = tclass##T<char>;\
	using tclass##16 = tclass##T<char16>;\
	using tclass##32 = tclass##T<char32>;

#define DEFINE_FULL_CHAR_CLASS(tclass) \
	template class tclass##T<char>;\
	template class tclass##T<char16>;\
	template class tclass##T<char32>;


namespace kr
{
	namespace meta
	{
		template <typename T>
		struct array_countof;

		template <typename T, size_t sz>
		struct array_countof<T[sz]>
		{
			static constexpr size_t value = sz;
		};
	}

}

constexpr size_t operator ""_sz(unsigned long long n)
{
	return n <= ((size_t)-1) ? (size_t)n : (throw std::logic_error("number overflow"));
}


#define countof(x)	(::kr::meta::array_countof<decltype(x)>::value)
#define endof(x)	((x) + countof(x))

#define UNPACK(...) __VA_ARGS__
#define _PRI_TOSTR(x) #x
#define TOSTR(x) _PRI_TOSTR(x)
#define _PRI_CONCAT(x,y) x##y
#define CONCAT(x,y) _PRI_CONCAT(x,y)
#define _PRI_CONCAT_CALL(x,y,...) UNPACK(x##y(__VA_ARGS__))
#define CONCAT_CALL(x,y,...) _PRI_CONCAT_CALL(x,y,__VA_ARGS__)

#define _PRI_TOUTF16(x) u##x
#define TOUTF16(x) _PRI_TOUTF16(x)
#define u__FILE__ TOUTF16(__FILE__)

#define _PRI_TOWIDE(x) L##x
#define TOWIDE(x) _PRI_TOWIDE(x)
#define L__FILE__ TOWIDE(__FILE__)

#define UNIQUE(name) CONCAT(__##name##__,__LINE__)

// varadic counter

#define _KR_PRI_INDEXOF32(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32, ...) a32
#define COUNTOF(...) UNPACK(_KR_PRI_INDEXOF32(__VA_ARGS__ ,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
/// for

#define FOR1(fn,x)		fn(x)
#define FOR2(fn,x,...)	fn(x), UNPACK(FOR1(fn,__VA_ARGS__))
#define FOR3(fn,x,...)	fn(x), UNPACK(FOR2(fn,__VA_ARGS__))
#define FOR4(fn,x,...)	fn(x), UNPACK(FOR3(fn,__VA_ARGS__))
#define FOR5(fn,x,...)	fn(x), UNPACK(FOR4(fn,__VA_ARGS__))
#define FOR6(fn,x,...)	fn(x), UNPACK(FOR5(fn,__VA_ARGS__))
#define FOR7(fn,x,...)	fn(x), UNPACK(FOR6(fn,__VA_ARGS__))
#define FOR8(fn,x,...)	fn(x), UNPACK(FOR7(fn,__VA_ARGS__))
#define FOR(fn,...)		UNPACK(CONCAT(FOR,COUNTOF(__VA_ARGS__))(fn,__VA_ARGS__))

#define RFOR1(fn,x)		fn(x)
#define RFOR2(fn,x,...)	UNPACK(RFOR1(fn,__VA_ARGS__)), fn(x)
#define RFOR3(fn,x,...)	UNPACK(RFOR2(fn,__VA_ARGS__)), fn(x)
#define RFOR4(fn,x,...)	UNPACK(RFOR3(fn,__VA_ARGS__)), fn(x)
#define RFOR5(fn,x,...)	UNPACK(RFOR4(fn,__VA_ARGS__)), fn(x)
#define RFOR6(fn,x,...)	UNPACK(RFOR5(fn,__VA_ARGS__)), fn(x)
#define RFOR7(fn,x,...)	UNPACK(RFOR6(fn,__VA_ARGS__)), fn(x)
#define RFOR8(fn,x,...)	UNPACK(RFOR7(fn,__VA_ARGS__)), fn(x)
#define RFOR(fn,...)	UNPACK(CONCAT(RFOR,COUNTOF(__VA_ARGS__))(fn,__VA_ARGS__))

#define RFORI1(fn,x)		fn(x,0)
#define RFORI2(fn,x,...)	UNPACK(RFORI1(fn,__VA_ARGS__)), fn(x,1)
#define RFORI3(fn,x,...)	UNPACK(RFORI2(fn,__VA_ARGS__)), fn(x,2)
#define RFORI4(fn,x,...)	UNPACK(RFORI3(fn,__VA_ARGS__)), fn(x,3)
#define RFORI5(fn,x,...)	UNPACK(RFORI4(fn,__VA_ARGS__)), fn(x,4)
#define RFORI6(fn,x,...)	UNPACK(RFORI5(fn,__VA_ARGS__)), fn(x,5)
#define RFORI7(fn,x,...)	UNPACK(RFORI6(fn,__VA_ARGS__)), fn(x,6)
#define RFORI8(fn,x,...)	UNPACK(RFORI7(fn,__VA_ARGS__)), fn(x,7)
#define RFORI(fn,...)	UNPACK(CONCAT(RFORI,COUNTOF(__VA_ARGS__))(fn,__VA_ARGS__))
#define _KR_PRI_FORI(fn,...)	UNPACK((fn, CONCAT(RFORI,COUNTOF(__VA_ARGS__))(UNPACK, __VA_ARGS__)))
#define FORI(fn,...)	CONCAT(RFORI,COUNTOF(__VA_ARGS__))_KR_PRI_FORI(fn, __VA_ARGS__)

#define RSEMI_FORI1(fn,x)		fn(x,0)
#define RSEMI_FORI2(fn,x,...)	UNPACK(RSEMI_FORI1(fn,__VA_ARGS__)); fn(x,1)
#define RSEMI_FORI3(fn,x,...)	UNPACK(RSEMI_FORI2(fn,__VA_ARGS__)); fn(x,2)
#define RSEMI_FORI4(fn,x,...)	UNPACK(RSEMI_FORI3(fn,__VA_ARGS__)); fn(x,3)
#define RSEMI_FORI5(fn,x,...)	UNPACK(RSEMI_FORI4(fn,__VA_ARGS__)); fn(x,4)
#define RSEMI_FORI6(fn,x,...)	UNPACK(RSEMI_FORI5(fn,__VA_ARGS__)); fn(x,5)
#define RSEMI_FORI7(fn,x,...)	UNPACK(RSEMI_FORI6(fn,__VA_ARGS__)); fn(x,6)
#define RSEMI_FORI8(fn,x,...)	UNPACK(RSEMI_FORI7(fn,__VA_ARGS__)); fn(x,7)
#define RSEMI_FORI(fn,...)	UNPACK(CONCAT(SEMI_RFORI,COUNTOF(__VA_ARGS__))(fn,__VA_ARGS__))
#define _KR_PRI_SEMI_FORI(fn,...)	UNPACK((fn, CONCAT(SEMI_RFORI,COUNTOF(__VA_ARGS__))(UNPACK, __VA_ARGS__)))
#define SEMI_FORI(fn,...)	CONCAT(RSEMI_FORI,COUNTOF(__VA_ARGS__))_KR_PRI_SEMI_FORI(fn, __VA_ARGS__)

#define _KR_PRI_FOR_TYPES(x,i)	x v##i
#define _KR_PRI_FOR_VALUES(x,i)	v##i
#define _KR_PRI_FOR_CTOR(x,i) v##i(v##i)

#define FOR_TYPES(...)		UNPACK(FORI(_KR_PRI_FOR_TYPES,__VA_ARGS__))
#define FOR_VALUES(...)		UNPACK(FORI(_KR_PRI_FOR_VALUES,__VA_ARGS__))
#define FOR_PROPERTIES(...)	UNPACK(SEMI_FORI(_KR_PRI_FOR_TYPES,__VA_ARGS__))
#define FOR_CTOR(...)		UNPACK(FORI(_KR_PRI_FOR_CTOR,__VA_ARGS__))

#define _kr_pri_throw_call_0 noexcept(false)
#define _kr_pri_throw_call_1 noexcept

#define _kr_pri_has_comma(...) UNPACK(_KR_PRI_INDEXOF32(__VA_ARGS__,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0))
#define _kr_pri_test_empty_check() ,
#define _kr_pri_test_empty_without_comma(...) _kr_pri_has_comma(_kr_pri_test_empty_check __VA_ARGS__ ())
#define _kr_pri_check_empty_flags_var_01 ,
#define _kr_pri_check_empty_flags_2(_1,_2) _kr_pri_has_comma(_kr_pri_check_empty_flags_var_##_1##_2)
#define _kr_pri_check_empty_flags(_1,_2) _kr_pri_check_empty_flags_2(_1, _2)
#define _kr_pri_test_empty(...) _kr_pri_check_empty_flags(_kr_pri_has_comma(__VA_ARGS__), _kr_pri_test_empty_without_comma(__VA_ARGS__))

#define throws(...) CONCAT(_kr_pri_throw_call_, _kr_pri_test_empty(__VA_ARGS__))

