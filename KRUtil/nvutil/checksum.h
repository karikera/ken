#pragma once

#include <KR3/main.h>

namespace kr
{
#ifndef NO_USE_FILESYSTEM
	int fileCheckSum(const char * strFileName) noexcept;
	int fileCheckSum(const char16 * strFileName) noexcept;
#endif

	word checkSum(void * ptr, size_t size) noexcept;

}


