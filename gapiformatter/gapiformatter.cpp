#include <KR3/main.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/util/wide.h>
#include <KR3/data/set.h>
#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <KR3/parser/parser.h>

using namespace kr;

class FormatParser:public Parser
{
private:
	Must<File> m_file;
	io::FOStream<char> m_fos;
	AText m_filename;
	Set<Text> m_altnamemap;
	int m_tab;
	bool m_base;

public:
	FormatParser(pcstr16 input, File * file, File * out) noexcept
		:Parser(file), m_filename(toAnsi((Text16)input)), m_file(file), m_fos(out)
	{
		m_tab = 0;
		m_base = false;
		m_altnamemap.insert("template");
		m_altnamemap.insert("if");
		m_altnamemap.insert("while");
		m_altnamemap.insert("for");
		m_altnamemap.insert("try");
		m_altnamemap.insert("catch");
		m_altnamemap.insert("throw");
		m_altnamemap.insert("small");
	}
	FormatParser(pcstr16 input, pcstr16 output)
		:FormatParser(input, File::open(input), File::create(output))
	{
	}

	Text filename() noexcept
	{
		return m_filename;
	}

	int parseFile()
	{
		m_fos << "#pragma once\n#include <KR3/main.h>\n#include <KRUtil/parser/jsonparser.h>";
		try
		{
			int count = 0;
			Text filename = path.filenameOnly(m_filename);
			for (Text name : filename.splitIterable('$'))
			{
				line() << "namespace " << name;
				open();
				count++;
			}
			parseNamespace();

			for (int i = 0; i < count; i++)
			{
				close();
			}
			line();
			return 0;
		}
		catch (InvalidSourceException&)
		{
			cerr << m_filename << "(" << m_line << "): parse error" << endl;
			return EBADF;
		}
	}

	void parseNamespace()
	{
		bool base = m_base;
		try
		{
			for (;;)
			{
				skipWhiteSpace();
				if (m_is.nextIs('}')) break;
				TSZ textline = m_is.readwith('{');
				Text lineread = textline.trim();

				Text type = lineread.readwith_y(Text::WHITE_SPACE);
				lineread.skipspace();
				if (type == nullptr)
				{
					if (lineread == "@base")
					{
						parseObjectType("__format_base");
						m_base = true;
					}
					else
					{
						parseObjectType(lineread);
					}
				}
				else if (type == "keyword")
				{
					m_altnamemap.insert(lineread);
				}
				else if (type == "namespace")
				{
					line() << "namespace " << lineread;
					open();
					parseNamespace();
					close();
				}
				else
				{
					throw InvalidSourceException();
				}
			}
		}
		catch (EofException&)
		{
		}
		m_base = base;
	}

	io::FOStream<char>& line()
	{
		m_fos << '\n' << fill('\t', m_tab);
		return m_fos;
	}

	void open()
	{
		line() << '{';
		m_tab++;
	}

	void close()
	{
		m_tab--;
		line() << '}';
	}

	void parseObjectType(Text name)
	{
		bool base = m_base;
		m_base = false;

		skipWhiteSpace();
		switch (m_is.peek())
		{
		case '"':
		case '}': {
			Array<bool> altFieldNames;
			altFieldNames.reserve(32);
			TmpArray<AText> fieldNames;
			line() << "struct " << name;
			if (base)
			{
				m_fos << " : __format_base";
			}
			open();
			for (;;)
			{
				skipWhiteSpace();
				if (m_is.nextIs('}')) break;

				m_is.must('"');
				AText fieldName = readWithUnslash('"');
				skipWhiteSpace();
				bool optional = m_is.nextIs('?');
				m_is.must(':');
				
				{
					AText defaultValue;
					TSZ typeName = parseType(fieldName, &defaultValue);
					line();
					if (optional) m_fos << "Keep<" << typeName << '>';
					else m_fos << typeName;
					m_fos << ' ' << fieldName;
					bool usealt;
					if ((usealt = m_altnamemap.has(fieldName)))
					{
						m_fos << '_';
					}
					altFieldNames.push(usealt);
					if (defaultValue != nullptr) m_fos << " = " << defaultValue;
					m_fos << ';';
				}

				fieldNames.push(move(fieldName));
			}

			line() << "void parseJson(JsonParser & parser)";
			open();
			line() << "parser.fields([this](JsonField& field)";

			open();
			auto altiter = altFieldNames.begin();
			for (Text fieldName : fieldNames)
			{
				line() << "field(\"" << addSlashes(fieldName) << "\", &" << fieldName;
				if (*altiter) m_fos << '_';
				m_fos << ");";
				altiter++;
			}
			close();
			line() << ");";
			close();
			close();
			m_fos << ';';
			break;
		}
		default:
			skipWhiteSpace();
			m_is.must("(key)");
			skipWhiteSpace();
			m_is.must(':');
			skipWhiteSpace();
			AText defaultValue;
			AText typeName = parseType(name, &defaultValue);
			m_is.must('}');

			line() << "using " << name << " = Map<Text, " << typeName << ">;";
			break;
		}

		m_base = base;
	}

	TSZ parseType(Text name, AText * defaultValue)
	{
		skipWhiteSpace();
		AText fieldComment;
		AText vartype;
		char starter = m_is.peek();
		switch (starter)
		{
		case '{': {
			m_is.skip();
			TSZ typeName;
			typeName << name;
			typeName << "_t";
			parseObjectType(typeName);
			skipWhiteSpace();
			m_is.nextIs(',');
			return typeName;
		}
		case '[': {
			m_is.skip();
			AText defaultValue;
			AText typeName = parseType(name, &defaultValue);
			skipWhiteSpace();
			m_is.must(']');

			TSZ out;
			out << "Array<" << typeName << '>';
			skipWhiteSpace();
			m_is.nextIs(',');
			return out;
		}
		}

		TSZ out;
		switch (starter)
		{
		case '"': {
			m_is.skip();
			fieldComment = readWithUnslash('"');
			out << "kr::AText";
			skipWhiteSpace();
			break;
		}
		default:
			AText vartype = m_is.readto_y(",}]=\r\n").trim();
			if (vartype == "etag" || vartype == "datetime")
			{
				fieldComment = move(vartype);
				out << "kr::AText";
				break;
			}
			if (vartype == "string")
			{
				out << "kr::AText";
			}
			else if (vartype == "integer" || vartype == "int")
			{
				out << "int";
				*defaultValue = "0";
			}
			else if (vartype == "unsigned integer" || vartype == "unsigned int")
			{
				out << "uint";
				*defaultValue = "0";
			}
			else if (vartype == "boolean" || vartype == "bool")
			{
				out << "bool";
				*defaultValue = "false";
			}
			else if (vartype == "unsigned long")
			{
				out << "uint64_t";
				*defaultValue = "0";
			}
			else
			{
				vartype.replace(&out, " ", "::");
			}
		}
		skipWhiteSpace();
		if (m_is.nextIs('='))
		{
			skipWhiteSpace();
			*defaultValue = m_is.readto_y(",}]\r\n").trim();
		}
		skipWhiteSpace();
		m_is.nextIs(',');
		if (fieldComment != nullptr)
		{
			out << " /** " << fieldComment << " */";
		}
		return out;
	}

};

int wmain(int argn, wchar_t ** args)
{
	if (argn < 3)
	{
		cerr << "gapiformatter [input] [output]" << endl;
		return EINVAL;
	}
	try
	{
		FormatParser parser(unwide(args[1]), unwide(args[2]));
		return parser.parseFile();
	}
	catch (Error&)
	{
		cerr << "error: Invalid file input: " << toAnsi((Text16)unwide(args[1])) << endl;
		return EINVAL;
	}
}

