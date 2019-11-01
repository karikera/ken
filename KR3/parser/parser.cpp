#include "stdafx.h"
#include "parser.h"

using namespace kr;

Parser::Parser(io::VIStream<char> is) noexcept
	:m_is(_new io::VIStream<char>(move(is)))
{
	m_line = 1;
}
bool Parser::isNextWhiteSpace()
{
	try
	{
		char chr = m_is.peek();
		if (chr == '\n')
		{
			m_line++;
			return true;
		}
		else if (kr::Text(Text::WHITE_SPACE).contains(chr))
		{
			return true;
		}
	}
	catch (EofException&)
	{
	}
	return false;
}
void Parser::skipWhiteSpace()
{
	while (isNextWhiteSpace()) m_is.skip(1);
}
void Parser::mustWhiteSpace() throws(InvalidSourceException)
{
	if (!isNextWhiteSpace()) throw InvalidSourceException();
	do { m_is.skip(1); } while (isNextWhiteSpace());
}
TSZ Parser::readToSpace()
{
	TSZ text;
	readToSpace(&text);
	return text;
}
TSZ Parser::readWithUnslash(char needle)
{
	TSZ text;
	readWithUnslash(&text, needle);
	return text;
}
void Parser::skipWithUnslash(char needle)
{
	for (;;)
	{
		m_is.skipto_L([&](Text text) {
			const char needles[] = { (char)'\\', needle };
			pcstr finded = text.find_y(Text(needles, countof(needles)));
			if (finded != nullptr)
				m_line += (int)text.cut(finded).count('\n');
			return finded;
		});

		if (m_is.read() == '\\')
		{
			m_is.skip();
		}
		else return;
	}
}
io::BufferedIStream<io::VIStream<char>, true> * Parser::is() noexcept
{
	return &m_is;
}
io::BufferedIStream<io::VIStream<char>, true> * Parser::operator->() noexcept
{
	return &m_is;
}

int Parser::getLine() noexcept
{
	return m_line;
}

