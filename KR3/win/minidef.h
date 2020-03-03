#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <intsafe.h>

typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;
