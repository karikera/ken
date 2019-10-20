#include "stdafx.h"
#include "codepoint.h"


using namespace kr;

template <>
void CodePoint<char, Charset::Utf8>::next(View<char> * text) noexcept
{
	char first = **text;
	if (first >= 0)
	{
		(*text) ++;
	}
	else if ((first & 0xe0) == 0xc0)
	{
		(*text) += 2;
	}
	else if ((first & 0xf0) == 0xe0)
	{
		(*text) += 3;
	}
	else if ((first & 0xf8) == 0xf0)
	{
		(*text) += 4;
	}
}
template <>
void CodePoint<char, Charset::EucKr>::next(View<char> * text) noexcept
{
	if (**text < 0)
	{
		(*text)++;
		if (text->empty()) return;
	}
	(*text)++;
}
template <>
void CodePoint<char16, Charset::Default>::next(View<char16> * text) noexcept
{
	if ((**text & 0xfc00) == 0xd800)
	{
		(*text)++;
		if (text->empty()) return;
	}
	(*text)++;
}
template <>
void CodePoint<char32, Charset::Default>::next(View<char32> * text) noexcept
{
	(*text)++;
}
