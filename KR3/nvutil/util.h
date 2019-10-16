#pragma once

#include <tchar.h>

inline bool IsDotDirectory(const TCHAR * strFileName)
{
	if(strFileName[0] == _T('.'))
	{
		if(strFileName[1] == _T('.'))
		{
			if(strFileName[2] == _T('\0')) return true;
		}
		else if(strFileName[1] == _T('\0')) return true;
	}
	return false;
}