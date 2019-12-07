#include "stdafx.h"
#include <KR3/util/wide.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/parser/langparser.h>
#include <KR3/fs/file.h>

#include <KR3/data/map.h>

using namespace kr;

LangParser fis(LangParser::DEFAULT_WORD_SPLITTER);
LangWriter fos;

char parseValue(Text endCode)
{
	AText value;
	TmpArray<char> brackets;
	for (;;)
	{
		value.clear();
		char oper = fis.skipSpace();
		if (oper == ' ')
		{
			oper = fis.readWord(&value);
		}
		if (brackets.empty())
		{
			if (endCode.contains(oper))
			{
				fos << value << endl;
				return oper;
			}
		}
		else
		{
			if (brackets.back() == oper)
			{
				brackets.pop();
			}
		}
		switch (oper)
		{
		case ' ': 
			if (value == "new") continue;
			break;
		case '{': brackets.push('}'); break;
		case '(': brackets.push(')'); break;
		case '[': brackets.push(']'); break;
		}
		fos << value << oper;
	}
}

int CT_CDECL wmain(int argn, wchar_t ** args)
{
	if (argn < 3)
	{
		cout << "jstoh [input] [output]" << endl;
		return EINVAL;
	}

	pcstr16 filename = unwide(args[1]);
	pcstr16 outfilename = unwide(args[2]);
	try
	{
		Must<File> ifile = File::open(filename);
		Must<File> ofile = File::create(outfilename);
		fis.setStream((File*)ifile);
		fos.setStream((File*)ofile);
		fos << "#pragma once" << endl << endl;
		while (!fis.eof())
		{
			AText def = fis.readWord(' ');
			if (def.empty()) continue;
			if (def == "var" || def == "const" || def == "let")
			{
				AText varname = fis.readWord('=');
				switch (fis.skipSpace())
				{
				case '{':
				{
					fis.get();

					AText property;
					if (fis.readWord(&property, ":}") != '}')
					{
						for (;;)
						{
							fos << "#define " << varname << '_' << property << ' ';
							if (parseValue(",}") == '}') break;
							property.clear();
							if (fis.readWord(&property, ":}") == '}') break;
						}
						fis.skipSpace(';');
					}
					break;
				}
				case ' ':
				{
					fos << "#define " << varname << ' ';
					parseValue(";");
					break;
				}
				}

			}
			else
			{
				ucerr << filename << u'(' << fis.getLine() << u"): error unknown var type(" << ansiToUtf16(def) << u")" << endl;
			}
		}
		fos.flush();
		return 0;
	}
	catch (InvalidSourceException&)
	{
		ucerr << filename << u'(' << fis.getLine() << u"): Invalid" << endl;
	}
	catch (LangParser::UnclosedCommentException&)
	{
		cerr << "Comment unclosed" << endl;
	}
	catch (LangParser::UnexpectedCharacterException& e)
	{
		ucerr << filename << u'(' << fis.getLine() << u"): Unexpected character " << (char16)e.actually << u" (respected: " << ansiToUtf16(e.respected) << u")" << endl;
	}
	catch (...)
	{
		ucerr << filename << u'(' << fis.getLine() << u"): Exception occured" << endl;
	}

	kr::File::remove(outfilename);
	return EINVAL;
}

