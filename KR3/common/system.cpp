#include "stdafx.h"
#include "system.h"


#ifdef WIN32

#include "wl/windows.h"

namespace
{
	struct SystemInfo:SYSTEM_INFO
	{
		SystemInfo() noexcept
		{
			GetSystemInfo(this);
		}

		static SystemInfo* getInstance() noexcept
		{
			static SystemInfo info;
			return &info;
		}
	};
}


kr::dword kr::getAllocationGranularity() noexcept
{
	return SystemInfo::getInstance()->dwAllocationGranularity;
}
kr::dword kr::getCPUCount() noexcept
{
	return SystemInfo::getInstance()->dwNumberOfProcessors;
}
#endif