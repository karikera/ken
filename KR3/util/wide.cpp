#include "stdafx.h"
#include "wide.h"


namespace kr
{
	const wchar_t* szlize(Text16 text, TText16* buffer) noexcept
	{
		try
		{
			if (*text.end() == '\0') return wide(text.data());
		}
		catch (...)
		{
		}
		buffer->clear();
		*buffer << text;
		return wide(buffer->c_str());
	}
}