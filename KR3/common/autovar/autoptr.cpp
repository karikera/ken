#include "stdafx.h"
#include "autoptr.h"


template class kr::autoptr_t<uintptr_t>;
#ifndef _WIN64
template class kr::autoptr_t<uint64_t>;
#endif