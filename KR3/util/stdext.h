#pragma once

#include <string>

namespace std
{
	namespace krext
	{
		template <typename _Elem>
		basic_string<_Elem> trim(const basic_string<_Elem> & s);

		template <typename _Elem>
		void toLowerCase(basic_string<_Elem> & str);

		template <typename _Elem>
		basic_string<_Elem>& replaceAll(basic_string<_Elem> &str,
			_Elem pattern, _Elem replace);
		template <typename _Elem>
		basic_string<_Elem>& replaceAll(basic_string<_Elem> &str,
			const basic_string<_Elem> &pattern,
			const basic_string<_Elem> &replace);

	}
}
