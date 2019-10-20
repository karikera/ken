#include <KR3/main.h>
#include <KR3/fs/installer.h>
#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <iostream>

using namespace kr;

Text16 unwrapQuot(char16 * text) noexcept
{
	if (*text == '\"') text++;
	char16 * textend = mem16::find(text, '\0') - 1;
	if (*textend == '\"') *textend-- = '\0';
	if (*textend == '\\') *textend-- = '\\';
	else if (*textend == '/') *textend-- = '/';
	return Text16(text, textend+1);
}

int CT_CDECL wmain(int argn, char16 ** args)
{
	if (argn < 3)
	{
		cerr << "Need more parameters." << endl;
		return EINVAL;
	}

	{
		std::locale kor("korean");
		std::wcout.imbue(kor);
		std::wcerr.imbue(kor);
	}

	Text16 src = unwrapQuot(args[1]);
	Text16 dest = unwrapQuot(args[2]);
	
	if (File::isDirectory(src.data()))
	{
		if (argn >= 4)
		{
			Text16 regexp = unwrapQuot(args[3]);
			Installer installer(dest, src, regexp);
			installer.all();
			ucout << u"Copied " << installer.m_copyCount << u", Failed " << installer.m_errorCount << u", Latest " << installer.m_skipCount << endl;
		}
		else
		{
			Installer installer(dest, src);
			installer.all();
			ucout << u"Copied " << installer.m_copyCount << u", Failed " << installer.m_errorCount << u", Latest " << installer.m_skipCount << endl;
		}
	}
	else
	{
		TSZ16 ndest;

		if (File::isDirectory(dest.data()))
		{
			ndest << dest << u'\\' << path16.basename(src);
			ndest.c_str();
			dest = ndest;
		}
		int latest = 0;
		int errored = 0;
		int copyed = 0;
		try
		{
			filetime_t src_modified = File::getLastModifiedTime(src.data());
			filetime_t dest_modified = File::getLastModifiedTime(dest.data());
			if (src_modified > dest_modified) throw 0;
			latest++;
		}
		catch (...)
		{
			if (File::copy(dest.data(), src.data()))
			{
				copyed++;
			}
			else
			{
				errored++;
			}
		}
		
		ucout << u"Copied " << copyed << u", Failed " << errored << u", Latest " << latest << endl;
	}
}