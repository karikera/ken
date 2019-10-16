#include "stdafx.h"
#include "csvparser.h"

#include <KR3/meta/text.h>

kr::CSVParser::CSVParser(nullptr_t) noexcept
	:m_stream(nullptr)
{
	m_nLast = ',';
	m_line = 1;
}

kr::CSVParser::CSVParser(io::VIStream<char> stream)
	:m_stream(_new io::VIStream<char>(stream))
{
	m_nLast = ',';
	m_line = 1;
	m_stream.hasBom();
}

kr::CSVParser& kr::CSVParser::operator = (nullptr_t)
{
	m_stream = nullptr;
	return *this;
}

kr::CSVParser& kr::CSVParser::operator = (io::VIStream<char> stream)
{
	m_stream.close();
	m_stream.resetIStream(_new io::VIStream<char>(stream));
	return *this;
}

kr::io::VIStream<char>* kr::CSVParser::operator &() const
{
	return m_stream.base();
}


size_t kr::CSVParser::nextLength() noexcept
{
	size_t len = 0;
	try
	{
		char chr = m_stream.peek();
		size_t start = 0;
		if (chr == '\"')
		{
			start++;
			size_t pos = start;
			for (;;)
			{
				try
				{
					pos = m_stream.pos('\"', pos);
					if (m_stream.peekAt(pos + 1) != '\"')
					{
						len += pos - start;
						start = pos + 1;
						break;
					}
					else
					{
						pos++;
						len += pos - start;
						pos++;
						start = pos;
					}
				}
				catch (EofException&)
				{
					return len;
				}
			}
		}

		size_t pos = m_stream.pos_y(",\n", start);
		return pos - start + len;
	}
	catch (EofException&)
	{
		return eof;
	}
}

void kr::CSVParser::mustLine() throws(EofException, NoLineException)
{
	switch (m_nLast)
	{
	case '\n': m_nLast = ','; break;
	case eof: throw EofException();
	default: throw NoLineException();
	}
	m_line++;
}

bool kr::CSVParser::nextLine() noexcept
{
	if (m_nLast == '\n')
	{
		m_nLast = ',';
		m_line++;
		return true;
	}
	try
	{
		char finded;
		do
		{
			char chr = m_stream.peek();
			if (chr == '\"')
			{
				m_stream.skip(1);

				size_t pos = 0;
				for (;;)
				{
					try
					{
						pos = m_stream.pos('\"', pos);
						pos++;
						if (m_stream.peekAt(pos) != '\"')
						{
							m_stream.skip(pos);
							break;
						}
						else
						{
							m_stream.skip(pos + 1);
							pos = 0;
						}
					}
					catch (TooBigException&)
					{
						m_stream.clearBuffer();
					}
				}
			}

			m_stream.readwith_y(",\n", &finded);
		} while (finded == ',');
		m_nLast = ',';
		m_line++;
		return true;
	}
	catch (EofException&)
	{
		m_stream.clearBuffer();
		m_nLast = eof;
		return false;
	}
}

int kr::CSVParser::getLine() noexcept
{
	return m_line;
}

void kr::CSVParser::skip() throws(EofException, NoLineException, NextLineException)
{
	switch (m_nLast)
	{
	case eof: throw EofException();
	case '\n':
		m_nLast = ',';
		m_line++;
		throw NextLineException();
	case ',': break;
	}

	int finded = 0;

	try
	{
		while (m_stream.peek() == '\"')
		{
			m_stream.skip(1);
			m_stream.skipwith('\"');
		}

		finded = m_stream.skipwith_y(",\n");
	}
	catch (EofException&)
	{
		m_stream.clearBuffer();
		finded = eof;
	}

	m_nLast = finded;
}

kr::TmpArray<char> kr::CSVParser::next() throws(EofException, NoLineException, NextLineException)
{
	TText tx((size_t)0, 1024);
	next(&tx);
	return tx;
}

kr::Array<char> kr::CSVParser::nextAlloc() throws(EofException, NoLineException, NextLineException)
{
	AText tx;
	next(&tx);
	tx.shrink();
	return tx;
}

kr::CSVWriter::CSVWriter(nullptr_t) noexcept
{
	m_prev = false;
}

kr::CSVWriter::CSVWriter(io::VOStream<char> stream) noexcept 
	: m_stream(stream)
{
	m_prev = false;
}

kr::CSVWriter& kr::CSVWriter::operator = (nullptr_t) noexcept
{
	m_stream.reset();
	return *this;
}

kr::CSVWriter& kr::CSVWriter::operator = (io::VOStream<char> stream) noexcept
{
	m_stream = stream;
	return *this;
}

kr::io::VOStream<char>* kr::CSVWriter::operator &() noexcept
{
	return &m_stream;
}


void kr::CSVWriter::write(View<char> str) throws(NotEnoughSpaceException)
{
	write<View<char>, char>(str);
}

void kr::CSVWriter::nextLine() throws(NotEnoughSpaceException)
{
	m_stream.write('\n');
	m_prev = false;
}
