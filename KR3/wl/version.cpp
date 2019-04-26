#include "stdafx.h"

#ifdef WIN32

#include "version.h"
#include "windows.h"

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8                   0x0602
#endif
#ifndef _WIN32_WINNT_WINBLUE
#define _WIN32_WINNT_WINBLUE                0x0603
#endif
#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10                  0x0A00
#endif
#include "vhelper.h"


kr::TSZ16 kr::getWindowsVersionText() noexcept
{
	TSZ16 version;
	if (IsWindows10OrGreater())
		version << u"Windows 10";
	else if (IsWindows8Point1OrGreater())
		version << u"Windows 8.1";
	else if (IsWindows8OrGreater())
		version << u"Windows 8";
	else if (IsWindows7SP1OrGreater())
		version << u"Windows 7 SP1";
	else if (IsWindows7OrGreater())
		version << u"Windows 7";
	else if (IsWindowsVistaSP2OrGreater())
		version << u"Windows Vista SP2";
	else if (IsWindowsVistaSP1OrGreater())
		version << u"Windows Vista SP1";
	else if (IsWindowsVistaOrGreater())
		version << u"Windows Vista";
	else if (IsWindowsXPSP3OrGreater())
		version << u"Windows XP SP3";
	else if (IsWindowsXPSP2OrGreater())
		version << u"Windows XP SP2";
	else if (IsWindowsXPSP1OrGreater())
		version << u"Windows XP SP1";
	else if (IsWindowsXPOrGreater())
		version << u"Windows XP";
	else
		version << u"Under Windows XP";

	if (IsWindowsServer())
		version << u" Server";

	LONG(WINAPI *pfnRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
	(FARPROC&)pfnRtlGetVersion = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlGetVersion");
	if (pfnRtlGetVersion)
	{
		RTL_OSVERSIONINFOEXW ver = { 0 };
		ver.dwOSVersionInfoSize = sizeof(ver);

		if (pfnRtlGetVersion(&ver) == 0)
		{
			version << u" (ID " << ver.dwPlatformId << u')';
			version << u" (Version " << ver.dwMajorVersion << u'.' << ver.dwMinorVersion << u')';
			version << u" (SP " << ver.wServicePackMajor << u'.' << ver.wServicePackMinor << u')';
			version << u" (Build " << ver.dwBuildNumber << u')';
		}
	}

	return version;
}

#endif