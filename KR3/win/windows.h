#pragma once

#ifndef WIN32
#error is not windows system
#endif

#ifndef _WINDOWS_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <KR3/util/wide.h>
#include <KR3/util/path.h>

#endif