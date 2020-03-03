#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/main.h>

namespace kr
{
	TSZ16 getWindowsVersionText() noexcept;
}


