#pragma once

#include "handle.h"

namespace kr
{
	namespace win
	{
		bool loadFileDialog(Window* hWnd, const char* Title, Text Type, Text TypeName, char* strFileName) noexcept;
		bool saveFileDialog(Window* hWnd, const char* Title, Text Type, Text TypeName, char* strFileName) noexcept;

		bool loadFileDialog(Window* hWnd, const char16* Title, Text16 Type, Text16 TypeName, char16* strFileName) noexcept;
		bool saveFileDialog(Window* hWnd, const char16* Title, Text16 Type, Text16 TypeName, char16* strFileName) noexcept;
	}
}