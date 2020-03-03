#include "stdafx.h"
#include "singleinstance.h"

#ifdef WIN32
#include <KR3/win/windows.h>
#endif

bool kr::singleInstanceTest() noexcept
{
#ifdef WIN32
	static HANDLE mutex;
	static finally{
		// The app is closing so release
		// the mutex.
		ReleaseMutex(mutex);
	};

	WCHAR filepath[MAX_PATH];
	GetModuleFileNameW(nullptr, filepath, MAX_PATH);

	WCHAR * filename = wcsrchr(filepath, L'\\');

	// Try to open the mutex.
	mutex = OpenMutexW(MUTEX_ALL_ACCESS, 0, filename);
	if (mutex)
	{
		mutex = nullptr;
		return false;
	}

	// Mutex doesn¡¯t exist. This is
	// the first instance so create
	// the mutex.
	mutex = CreateMutexW(0, 0, filename);
	return true;
#else
	notImplementedYet();
#endif
}
