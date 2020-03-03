#include <KR3/main.h>
#include <KR3/fs/installer.h>
#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <KR3/util/parameter.h>
#include <iostream>

#include <KR3/win/windows.h>

using namespace kr;

int CT_CDECL wmain(int argn, char16 ** args)
{
	if (argn < 3)
	{
		cerr << "Need more parameters." << endl;
		return EINVAL;
	}
	setlocale(LC_ALL, nullptr);

	Text16 src = (Text16)args[1];
	Text16 dest = (Text16)args[2];
	
	int error;

	if (path16.endsWithSeperator(src) || File::isDirectory(src.data()))
	{
		if (argn >= 4)
		{
			Text16 regexp = (Text16)args[3];
			Installer installer(dest, src, regexp);
			installer.all();
			if (installer.m_errorCount != 0) error = GetLastError();
			else error = S_OK;
			ucout << u"Copied " << installer.m_copyCount << u", Failed " << installer.m_errorCount << u", Latest " << installer.m_skipCount << endl;
		}
		else
		{
			Installer installer(dest, src);
			installer.all();
			if (installer.m_errorCount != 0) error = GetLastError();
			else error = S_OK;
			ucout << u"Copied " << installer.m_copyCount << u", Failed " << installer.m_errorCount << u", Latest " << installer.m_skipCount << endl;
		}
	}
	else
	{
		TSZ16 ndest;

		if (path16.endsWithSeperator(dest))
		{
			ndest << dest << path16.basename(src) << nullterm;
			dest = ndest;
		}
		else if (File::isDirectory(dest.data()))
		{
			ndest << dest << u'\\' << path16.basename(src) << nullterm;
			dest = ndest;
		}
		try
		{
			filetime_t src_modified = File::getLastModifiedTime(src.data());
			filetime_t dest_modified = File::getLastModifiedTime(dest.data());
			if (src_modified > dest_modified) throw 0;
			ucout << path16.basename(src) << u" latest" << endl;

			error = S_OK;
		}
		catch (...)
		{
			if (File::copy(dest.data(), src.data()))
			{
				ucout << path16.basename(src) << u" copied" << endl;
				error = S_OK;
			}
			else
			{
				error = GetLastError();
				if (error != ERROR_PATH_NOT_FOUND)
				{
					ucerr << path16.basename(src) << u" failed" << endl;
				}
				else
				{
					File::createFullDirectory(path16.dirname(dest));
					if (File::copy(dest.data(), src.data()))
					{
						ucout << path16.basename(src) << u" copied" << endl;
						error = S_OK;
					}
					else
					{
						ucerr << path16.basename(src) << u" failed" << endl;
						error = GetLastError();
					}
				}
			}
		}
	}
	return error;
}