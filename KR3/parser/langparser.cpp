#include "stdafx.h"
#include "langparser.h"

using namespace kr;

const char LangParser::DEFAULT_WORD_SPLITTER[] = ";:{}[](),.@=!-+*%^&|\\";

LangParser::UnexpectedCharacterException::UnexpectedCharacterException(char actually, char repected) noexcept
	:actually(actually), respectedBuffer{repected}, respected(respectedBuffer, 1)
{
}
LangParser::UnexpectedCharacterException::UnexpectedCharacterException(char actually, Text repected) noexcept
	: actually(actually), respected(repected)
{
}

void LangParser::test(char actually, Text respected) throws(UnexpectedCharacterException)
{
	if (respected.contains(actually)) return;
	throw UnexpectedCharacterException(actually, respected);
}
void LangParser::test(char actually, char respected) throws(UnexpectedCharacterException)
{
	if (actually == respected) return;
	throw UnexpectedCharacterException(actually, respected);
}

LangParser::LangParser() noexcept
	:m_is(nullptr)
{
	m_line = 1;
	m_eof = false;
}
LangParser::LangParser(Text wordSplitters) noexcept
	:LangParser()
{
	setWordSplitters(wordSplitters);
}
void LangParser::setWordSplitters(Text wordSplitters) noexcept
{
	m_splitters.insert(wordSplitters.begin(), wordSplitters.end());
}
bool LangParser::isSplitter(char chr) noexcept
{
	return m_splitters.find(chr) != m_splitters.end();
}
uint LangParser::getLine() noexcept
{
	return m_line;
}
bool LangParser::eof() noexcept
{
	return m_eof;
}

void LangParser::setStream(io::VIStream<char> is) noexcept
{
	m_is.close();
	m_is.resetIStream(_new io::VIStream<char>(is));
}

char LangParser::get()
{
	return m_is.read();
}
char LangParser::peek()
{
	return m_is.peek();
}

char LangParser::skipLine()
{
	m_is.skipLine();
	m_line++;
	return skipSpace();
}
char LangParser::skipComments()
{
	for (;;)
	{
		switch (m_is.read())
		{
		case '\n': m_line++; break;
		case '*':
			try
			{
				if (m_is.read() == '/') return skipSpace();
			}
			catch (EofException&)
			{
				m_eof = true;
				throw UnclosedCommentException();
			}
			break;
		}
	}
}
char LangParser::skipSpace()
{
	try
	{
		for (;;)
		{
			char chr = m_is.peek();
			switch (chr)
			{
			case '\n': m_line++;
			case '\r': case ' ': case '\t':
				m_is.read();
				break;
			case '/':
				switch (m_is.peek())
				{
				case '/':
					m_is.read();
					return skipLine();
				case '*':
					m_is.read();
					return skipComments();
				}
			default:
				if (isSplitter(chr))
				{
					m_is.read();
					return chr;
				}
				return ' ';
			}
		}
	}
	catch (EofException&)
	{
		m_eof = true;
		return ' ';
	}
}
void LangParser::skipSpace(char chr)
{
	test(skipSpace(), chr);
}
char LangParser::readWord(TText * out)
{
	try
	{
		skipSpace(' ');

		for (;;)
		{
			char chr = m_is.read();
			switch (chr)
			{
			case '\n': m_line++;
			case '\r': case ' ': case '\t':
				return skipSpace();
			case '/':
				switch (m_is.peek())
				{
				case '/':
					return skipLine();
				case '*':
					return skipComments();
				}
			default:
				if (isSplitter(chr))
				{
					return chr;
				}
				*out << chr;
				break;
			}
		}
	}
	catch (EofException&)
	{
		m_eof = true;
		return ' ';
	}
}
char LangParser::readWord(AText * out)
{
	TText text;
	char last = readWord(&text);
	*out = text;
	return last;
}
AText LangParser::readWord(char endCode)
{
	AText out;
	test(readWord(&out), endCode);
	return out;
}
int LangParser::readNumber(char endCode)
{
	TText text;
	char last = readWord(&text);
	test(last, endCode);
	return text.trim().to_int();
}
char kr::LangParser::skipSpace(Text mask)
{
	char det = skipSpace();
	test(det, mask);
	return det;
}
char kr::LangParser::readWord(AText * out, Text mask)
{
	char det = readWord(out);
	test(det, mask);
	return det;
}

LangWriter::LangWriter()
	:m_os(nullptr)
{
	m_indent = 0;
	m_endl = false;
}
void LangWriter::setStream(io::VOStream<char> os) noexcept
{
	m_os.resetStream(_new io::VOStream<char>(os));
}
void LangWriter::indent(int n) noexcept
{
	m_indent += n;
}
void LangWriter::flush() noexcept
{
	m_os.flush();
}
void LangWriter::writeImpl(const char * data, size_t size)
{
	auto iterable = Text(data, size).splitIterable('\n');
	auto iter = iterable.begin();
	auto end = iterable.end();
	_writeText(*iter++);
	while (iter != end)
	{
		m_os.write("\r\n");
		m_endl = true;
		_writeText(*iter++);
	}
}
void LangWriter::_writeText(Text text) noexcept
{
	if (text.empty()) return;
	if (m_endl)
	{
		for (size_t i = 0; i<m_indent; i++) m_os << ' ';
		m_endl = false;
	}
	m_os.write(text);
}
