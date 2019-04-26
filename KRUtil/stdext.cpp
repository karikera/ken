#include "stdafx.h"
#include "stdext.h"

#include <algorithm>

using namespace std;

#define OCTNUM '0': case '1': case '2': case '3': case '4':\
	case '5': case '6': case '7'
#define DECNUM OCTNUM: case '8': case '9'
#define HEXEXTRA 'a': case 'b': case 'c':\
	case 'd': case 'e': case 'f'
#define HEXEXTRA_UP 'A': case 'B': case 'C':\
	case 'D': case 'E': case 'F'

template <typename _Elem>
basic_string<_Elem> krext::trim(const basic_string<_Elem> & s)
{
	auto isnotempty = [](int chr) { return chr != ' ' && chr != '\t' && chr != '\n' && chr != '\r'; };
	auto beg = std::find_if(s.begin(), s.end(), isnotempty);
	auto end = std::find_if(s.rbegin(), s.rend(), isnotempty).base();
	return s.substr(beg - s.begin(), end - beg);
}

template <typename _Elem>
void krext::toLowerCase(basic_string<_Elem> & str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

template <typename _Elem>
basic_string<_Elem>& krext::replaceAll(basic_string<_Elem> &str,
	_Elem pattern, _Elem replace)
{
	transform(str.begin(), str.end(), str.begin(), [pattern, replace](_Elem chr) {
		return chr == pattern ? replace : chr;
	});
	return str;
}
template <typename _Elem>
basic_string<_Elem>& krext::replaceAll(basic_string<_Elem> &str,
	const basic_string<_Elem> &pattern,
	const basic_string<_Elem> &replace)
{
	using string = basic_string<_Elem>;
	typename string::size_type pos = 0;
	typename string::size_type offset = 0;

	while ((pos = str.find(pattern, offset)) != string::npos)
	{
		str.replace(str.begin() + pos, str.begin() + pos + pattern.size(), replace);
		offset = pos + replace.size();
	}
	return str;
}


template basic_string<char> krext::trim<char>(const basic_string<char> & s);
template void krext::toLowerCase<char>(basic_string<char> & str);

template basic_string<char>& krext::replaceAll(basic_string<char> &str,
	char pattern, char replace);
template basic_string<char>& krext::replaceAll(basic_string<char> &str,
	const basic_string<char> &pattern,
	const basic_string<char> &replace);

template basic_string<char16> krext::trim<char16>(const basic_string<char16> & s);
template void krext::toLowerCase<char16>(basic_string<char16> & str);

template basic_string<char16>& krext::replaceAll(basic_string<char16> &str,
	char16 pattern, char16 replace);
template basic_string<char16>& krext::replaceAll(basic_string<char16> &str,
	const basic_string<char16> &pattern,
	const basic_string<char16> &replace);
