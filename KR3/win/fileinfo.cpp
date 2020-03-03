#include "stdafx.h"

#ifdef WIN32
#include "fileinfo.h"

#include <KR3/win/windows.h>

#pragma comment(lib, "version.lib")


kr::ProgramVersion::ProgramVersion(qword abcd) noexcept
	:abcd(abcd)
{
}

kr::ProgramVersion::ProgramVersion(dword ab, dword cd) noexcept
	:ab(ab), cd(cd)
{
}

kr::ProgramVersion::ProgramVersion(word a, word b, word c, word d) noexcept
	:a(a), b(b), c(c), d(d)
{
}


kr::ProgramVersion kr::ProgramVersion::fromFile(pcstr16 filename) noexcept
{
	ProgramVersion out;

	DWORD handle;
	DWORD size = GetFileVersionInfoSizeW(wide(filename), &handle);
	if (size == 0)
	{
		return 0;
	}

	TmpArray<byte> filedata(size);
	if (!GetFileVersionInfoW(wide(filename), handle, size, filedata.begin()))
	{
		return 0;
	}

	VS_FIXEDFILEINFO * version;
	UINT verSize;
	if (!VerQueryValueW(filedata.begin(), L"\\", (VOID **)&version, &verSize))
	{
		out.abcd = 0;
		return out;
	}

	out.ab = version->dwFileVersionMS;
	out.cd = version->dwFileVersionLS;
	return out;
}

#endif