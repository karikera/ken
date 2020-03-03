#pragma once

#include <KR3/main.h>
#include <KR3/win/windows.h>

namespace kr
{
	// Define WindowStruct
	template <typename T> struct __WINAPI_T{};

#define WINAPI_STRUCTS() \
	_struct(WNDCLASSEX);

	template <> struct __WINAPI_T<char>
	{
#define _struct(x) using x##_ = x##A;
		WINAPI_STRUCTS();
#undef _struct
	};

	template <> struct __WINAPI_T<char16>
	{
#define _struct(x) using x##_ = x##W;
		WINAPI_STRUCTS();
#undef _struct
	};

#define _struct(x) template <typename T> using x##_T = typename __WINAPI_T<T>::x##_;
	WINAPI_STRUCTS();
#undef _struct

#undef WINAPI_STRUCTS

}