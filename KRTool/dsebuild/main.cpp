
#include "root.h"
#include "fstream.h"

#include <iostream>

#include <KR3/wl/windows.h>
#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <KR3/util/stdext.h>

using namespace std::krext;

int CT_CDECL wmain(int count, const wchar_t ** args)
{
	if(count < 2)
	{
		cerr << "Need more parameters." << endl;
		return EINVAL;
	}
	try
	{
		pcstr16 * args16 = unwide(args);
		ucout << args16[1] << endl;

		g_filename.reserve(File::NAMELEN);
		g_filename << currentDirectory << u'\\' << args16[1];
		g_filename.toLowerCase();

		Keep<File> ifile;
		Keep<File> ofile;
		try
		{
			ifile = File::open(g_filename.c_str());
			g_is.setStream((File*)ifile);
		}
		catch (...)
		{
			throw ErrMessage("File cannot read");
		}
		g_root.readClassContents('\xff');

		try
		{
			ofile = File::create(TSZ16() << g_filename << u".h");
			g_os.setStream((File*)ofile);
		}
		catch (...)
		{
			throw ErrMessage("File cannot write");
		}
		g_root.writeClassContents();
		g_os.flush();
	}
	catch (LangParser::UnclosedCommentException&)
	{
		ucerr << g_filename << u'(' << g_is.getLine() << u"): error Comment unclosed" << endl;
	}
	catch (LangParser::UnexpectedCharacterException& e)
	{
		ucerr << g_filename << u'(' << g_is.getLine() << u"): error Unexpected character " << (char16)e.actually << u" (respected: " << ansiToUtf16(e.respected) << u')' << endl;
	}
	catch(ErrMessage& err)
	{
		err.print();
		return -1;
	}
	return 0;
}
