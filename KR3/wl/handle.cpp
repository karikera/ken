#include "stdafx.h"

#ifdef WIN32

#include "handle.h"
#include "windows.h"

void kr::_pri_::closeHandle(void * handle) noexcept
{
	::CloseHandle((HANDLE)handle);
}

#endif