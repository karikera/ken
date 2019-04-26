#pragma once

#define __KR3_INCLUDED

#ifdef WIN32

#ifndef WINVER
#define WINVER 0x0601 // _WIN32_WINNT_WIN7
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER
#endif
#include <SDKDDKVer.h>

#endif

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include <initializer_list>
#include <utility>
#include <type_traits>
#include <atomic>
#include <exception>
#include <stdexcept>

namespace kr
{
	using std::initializer_list;
	using std::move;
	using std::forward;
}

#include "common/compiler.h"
#include "common/macro.h"
#include "common/type.h"
#include "common/reference.h"
#include "common/autovar.h"
#include "common/exception.h"
#include <KRNew/alloc.h>
#include "common/system.h"
#include "common/finally.h"
#include "common/common.h"
#include "common/intrinsic.h"
#include "common/keep.h"
#include "common/memlib.h"
#include "common/math.h"
#include "common/random.h"
#include "common/temp.h"
#include "common/text.h"
#include "common/stdio.h"
#include "common/error.h"
#include "common/io/vstream.h"
