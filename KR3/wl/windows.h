#pragma once

#ifndef WIN32
#error is not windows system
#endif

#ifndef _WINDOWS_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../util/wide.h"

#endif