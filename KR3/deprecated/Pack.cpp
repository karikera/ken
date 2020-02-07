#include "stdafx.h"

#include <fstream>

EMPTY_SOURCE

using namespace kr;
using std::endl;

namespace
{
	const char Key[] = "LnlyRake";
	constexpr size_t KeyLen = sizeof(Key) - 1;
	const int shuffle[] = { 5,7,1,4,2,6,3,0 };
	const int shuffle_inv[] = { 7,2,4,6,3,0,5,1 };
}

//
//void makePack() noexcept
//{
//	char16 currentPath[MAX_PATH];
//	char16 strPath[MAX_PATH];
//	char16 strFileName[MAX_PATH];
//
//	GetCurrentDirectoryW(MAX_PATH, currentPath);
//	wcscpy_s(strPath, currentPath);
//	{
//		OPENFILENAMEW ofn =
//		{
//			sizeof(OPENFILENAMEW), 0, 0,
//			u"蒲 颇老\0*.krz\0葛电 颇老\0*.*\0\0", nullptr,
//			0, 1, strFileName, MAX_PATH, nullptr, 0, strPath,
//			u"蒲 积己",
//			OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, 0, 0,
//			u".krz", 0, nullptr, nullptr
//		};
//		if (!GetSaveFileNameW(&ofn)) return;
//	}
//
//	std::wofstream headerfile((std::wstring)strFileName + u".h");
//	File * file = File::create("");
//
//	Temp<File> file(strFileName);
//
//	strFileName[0] = '\0';
//
//	headerfile << u"namespace file" << endl;
//	headerfile << L'{' << endl;
//	headerfile << u"\tstruct FileData{ unsigned long long offset, size; };" << endl;
//	
//	for (;;)
//	{
//		{
//			wcscpy_s(strPath, currentPath);
//			OPENFILENAMEW ofn =
//			{
//				sizeof(OPENFILENAMEW), 0, 0,
//				u"葛电 颇老\0*.*\0\0", nullptr,
//				0, 1, strFileName, MAX_PATH, nullptr, 0, strPath,
//				u"蒲 肺靛",
//				OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_EXPLORER, 0, 0,
//				0, 0, nullptr, nullptr
//			};
//			if (!GetOpenFileNameW(&ofn)) break;
//		}
//
//		char16* FileName=strFileName;
//		bool Single = wcschr(FileName, L'\0')[1] == L'\0';
//		qword offset = 0;
//
//		for (;;)
//		{
//			if(!Single)
//			{
//				FileName = wcschr(FileName, L'\0');
//				FileName++;
//				if (*FileName == L'\0') break;
//			}
//			else
//			{
//				FileName = wcsrchr(strFileName, L'\\') + 1;
//			}
//
//			HANDLE itemfile=CreateFileW(FileName,GENERIC_READ,0,nullptr,
//				OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
//
//			LARGE_INTEGER filesize;
//			if (GetFileSizeEx(itemfile, &filesize))
//			{
//				mem16::change(FileName, L'.', L'_', wcslen(FileName));
//				headerfile << u"\tFileData " << FileName << u" = { ";
//				headerfile << offset << u", ";
//				headerfile << filesize.QuadPart <<  u" };" << endl;
//
//				offset += filesize.QuadPart;
//
//				file->write(itemfile, filesize.QuadPart);
//				CloseHandle(itemfile);
//			}
//			if (Single) break;
//		}
//	}
//	headerfile << L'}' << endl;
//}
//

//
//void File::write(HANDLE handle, qword filesize) noexcept
//{
//	DWORD ressize;
//	while (filesize != 0)
//	{
//		if (!ReadFile(handle, m_buffer, sizeof(m_buffer), &ressize, 0))
//		{
//			return;
//		}
//
//		{
//			char * ptr = m_buffer;
//			char * end = ptr + ressize;
//			while (ptr != end)
//			{
//				*ptr += Key[m_keypos];
//				m_keypos = (m_keypos + 1) % KeyLen;
//			}
//		}
//
//		WriteFile(m_file, m_buffer, ressize, &ressize, 0);
//		filesize -= ressize;
//	}
//}