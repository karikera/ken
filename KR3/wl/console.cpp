#include "stdafx.h"

#ifdef WIN32
#include "console.h"
#include "../wl/windows.h"
#include <fcntl.h>
#include <io.h>

void showConsole()
{
	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((intptr_t)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, nullptr, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((intptr_t)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, nullptr, _IONBF, 128);
	*stdin = *hf_in;
}

#endif