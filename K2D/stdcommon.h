#pragma once

#include <KR3/main.h>

namespace kr
{
	inline bool kr_fopen_read(FILE ** file, pcstr16 filename) noexcept
	{
#ifdef WIN32
		if (_wfopen_s(file, (const wchar_t*)filename, L"rb") != 0) return false;
#else
		*file = fopen(TSZ() << toUtf8((Text16)filename), "rb");
		if (*file == nullptr) return false;
#endif
		return true;
	}
	inline bool kr_fopen_write(FILE ** file, pcstr16 filename) noexcept
	{
#ifdef WIN32
		if (_wfopen_s(file, (const wchar_t*)filename, L"wb") != 0) return false;
#else
		*file = fopen(TSZ() << toUtf8((Text16)filename), "wb");
		if (*file == nullptr) return false;
#endif
		return true;
	}
}
