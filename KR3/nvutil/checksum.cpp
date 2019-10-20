#include "stdafx.h"
#include "checksum.h"

#ifndef NO_USE_FILESYSTEM
#include <KR3/wl/windows.h>

inline int fileCheckSum(HANDLE hFile) noexcept
{
	DWORD dwJunk;
	DWORD dwSize;
	int result;
	BYTE * pBuffer;

	if(hFile != nullptr && hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, &dwJunk);
		pBuffer = _new BYTE[dwSize];
		ReadFile(hFile,pBuffer,dwSize,&dwJunk,nullptr);
		CloseHandle(hFile);

		result = kr::checkSum(pBuffer,dwSize);
		delete pBuffer;

		return result;
	}
	return -1;
}
int kr::fileCheckSum(const char16* strFileName) noexcept
{
	HANDLE hFile = CreateFileW(wide(strFileName),GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
	if(hFile == INVALID_HANDLE_VALUE) return -1;
	return ::fileCheckSum(hFile);
}
int kr::fileCheckSum(const char * strFileName) noexcept
{	
	HANDLE hFile = CreateFileA(strFileName,GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
	return ::fileCheckSum(hFile);
}
#endif

kr::word kr::checkSum(void * ptr,size_t size) noexcept
{
	word sum = 0;
	word* p = (word*)ptr;
	word* end = p + size/2;
	for(;p!=end;p++) sum ^= *p;
	if(size & 1) sum ^= *(byte*)p;
	return sum;
}