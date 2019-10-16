#include "stdafx.h"
#include "dlgx.h"

#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <commdlg.h>

using namespace kr;
using namespace win;

bool kr::win::loadFileDialog(Window* hWnd, const char* Title, Text Type, Text TypeName, char* strFileName) noexcept
{
	TText pathes((size_t)0, MAX_PATH);
	TText types((size_t)0, 512);
	TText defext((size_t)0, 512);

	pathes << currentDirectory;
	types << TypeName << '\0' << Type << "\0葛电颇老\0*.*\0\0";
	defext << '.' << Type;

	OPENFILENAMEA ofn = { sizeof(OPENFILENAMEA), hWnd, 0,
		types.begin(), nullptr,
		0, 1, strFileName, MAX_PATH, nullptr, 0, pathes.begin(),
		Title,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 0, 0,
		defext.begin(), 0, nullptr, nullptr };
	if (!GetOpenFileNameA(&ofn)) return false;
	return true;
}
bool kr::win::saveFileDialog(Window* hWnd, const char* Title, Text Type, Text TypeName, char* strFileName) noexcept
{
	TText pathes((size_t)0, MAX_PATH);
	TText types((size_t)0, 512);
	TText defext((size_t)0, 512);

	pathes << currentDirectory;
	types << TypeName << '\0' << Type << "\0葛电颇老\0*.*\0\0";
	defext << '.' << Type;

	OPENFILENAMEA ofn = { sizeof(OPENFILENAMEA), hWnd, 0,
		types.begin(), nullptr,
		0, 1, strFileName, MAX_PATH, nullptr, 0, pathes.begin(),
		Title,
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, 0, 0,
		defext.begin(), 0, nullptr, nullptr };
	if (!GetSaveFileNameA(&ofn)) return false;
	return true;
}

bool kr::win::loadFileDialog(Window* hWnd, const char16* Title, Text16 Type, Text16 TypeName, char16* strFileName) noexcept
{
	TText16 pathes((size_t)0, MAX_PATH);
	TText16 types((size_t)0, 512);
	TText16 defext((size_t)0, 512);

	pathes << currentDirectory;
	types << TypeName << u'\0' << Type << u"\0葛电颇老\0*.*\0\0";
	defext << u'.' << Type << u'\0';

	OPENFILENAMEW ofn = {
		sizeof(OPENFILENAMEW), hWnd, 0,
		wide(types.data()), nullptr,
		0, 1, wide(strFileName), MAX_PATH, nullptr, 0, wide(pathes.data()),
		wide(Title),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 0, 0,
		wide(defext.data()), 0, nullptr, nullptr };
	if (!GetOpenFileNameW(&ofn)) return false;
	return true;
}
bool kr::win::saveFileDialog(Window* hWnd, const char16* Title, Text16 Type, Text16 TypeName, char16* strFileName) noexcept
{
	TText16 pathes((size_t)0, MAX_PATH);
	TText16 types((size_t)0, 512);
	TText16 defext((size_t)0, 512);

	pathes << currentDirectory;
	types << TypeName << u'\0' << Type << u"\0葛电颇老\0*.*\0\0";
	defext << u'.' << Type;

	OPENFILENAMEW ofn = {
		sizeof(OPENFILENAMEW), hWnd, 0,
		wide(types.begin()), nullptr,
		0, 1, wide(strFileName), MAX_PATH, nullptr, 0, wide(pathes.begin()),
		wide(Title),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, 0, 0,
		wide(defext.begin()), 0, nullptr, nullptr };
	if (!GetSaveFileNameW(&ofn)) return false;
	return true;
}