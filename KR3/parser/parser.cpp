#include "stdafx.h"
#include "parser.h"

using namespace kr;

Parser::Parser(io::VIStream<char> is) noexcept
	:m_is(_new io::VIStream<char>(move(is)))
{
	m_line = 1;
}
bool Parser::isNextWhiteSpace() throws(EofException)
{
	try
	{
		char chr = m_is.peek();
		if (chr == '\n')
		{
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
void Parser::skipWhiteSpace() throws(...)
{
	try
	{
		for (;;)
		{
			char chr = m_is.peek();
			if (chr == '\n')
			{
				m_line++;
				m_is.skip(1);
			}
			else if (kr::Text(Text::WHITE_SPACE).contains(chr))
			{
				m_is.skip(1);
			}
			else
			{
				return;
			}
		}
	}
	catch (EofException&)
	{
	}
}
void Parser::skipToY(Text needles) throws(EofException)
{
	m_is.skipto_L([&](Text text) {
		pcstr finded = text.find_y(needles);
		if (finded != nullptr)
			m_line += text.cut(finded).count('\n');
		return finded;
		});
}
void Parser::skipTo(Text needle) throws(EofException)
{
	m_is.skipto_L([&](Text text) {
		pcstr finded = text.find(needle);
		if (finded != nullptr)
			m_line += text.cut(finded).count('\n');
		return finded;
		});
}
void Parser::skipTo(char needle) throws(EofException)
{
	m_is.skipto_L([&](Text text) {
		pcstr finded = text.find(needle);
		if (finded != nullptr)
			m_line += text.cut(finded).count('\n');
		return finded;
		});
}
void Parser::skipToLine() throws(EofException)
{
	skipToY("\r\n");
}
void Parser::mustWhiteSpace() throws(InvalidSourceException)
{
	if (!isNextWhiteSpace()) throw InvalidSourceException();
	skipWhiteSpace();
}
TSZ Parser::readToSpace() throws(EofException)
{
	TSZ text;
	readToSpace(&text);
	return text;
}
TSZ Parser::readToLine() throws(EofException)
{
	TSZ text;
	readToLine(&text);
	return text;
}
TSZ Parser::readWithUnslash(char needle) throws(EofException)
{
	TSZ text;
	readWithUnslash(&text, needle);
	return text;
}
void Parser::skipWithUnslash(char needle) throws(EofException)
{
	const char needles[] = { (char)'\\', needle };

	for (;;)
	{
		skipToY(Text(needles, countof(needles)));
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

size_t Parser::getLine() noexcept
{
	return m_line;
}
void Parser::skip(size_t sz) throws(EofException)
{
	for (;;)
	{
		Text buf = m_is.getBuffer();
		if (buf.size() <= sz)
		{
			m_line += (buf.cut(sz).count('\n'));
			m_is.skip(sz);
			return;
		}
		m_line += (buf.count('\n'));
		sz -= buf.size();
		m_is.refill();
	}
}

