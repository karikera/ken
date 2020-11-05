#pragma once

#include <KR3/main.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/meta/text.h>

namespace kr
{
	class ParsingException
	{
	};
	class NextLineException :public ParsingException
	{
	};
	class NoLineException :public ParsingException
	{
	};

	class CSVParser
	{
	public:
		CSVParser(nullptr_t) noexcept;
		CSVParser(io::VIStream<char> stream);
		CSVParser(const CSVParser&) = delete;
		CSVParser& operator = (nullptr_t);
		CSVParser& operator = (io::VIStream<char> stream);
		kr::io::VIStream<char>* operator &() const;

		void skip() throws(EofException, NoLineException, NextLineException);
		template <class Derived, typename C, typename _Info>
		void next(OutStream<Derived, C, _Info>* dest) throws(EofException, NoLineException, NextLineException);
		TmpArray<char> next() throws(EofException, NoLineException);
		Array<char> nextAlloc() throws(EofException, NoLineException);
		size_t nextLength() noexcept;
		void mustLine() throws(EofException, NoLineException);
		bool nextLine() noexcept;
		int getLine() noexcept;
		bool hasNext() noexcept;

	private:
		io::SelfBufferedIStream<io::VIStream<char>, true> m_stream;
		int m_nLast;
		int m_line;

	};

	class CSVWriter
	{
	public:
		CSVWriter(nullptr_t) noexcept;
		CSVWriter(io::VOStream<char> stream) noexcept;
		CSVWriter(const CSVWriter&) = delete;
		CSVWriter& operator = (nullptr_t) noexcept;
		CSVWriter& operator = (io::VOStream<char> stream) noexcept;
		io::VOStream<char>* operator &() noexcept;

		template <typename Converter, typename C2>
		void write(View<C2> str) throws(NotEnoughSpaceException);
		void write(View<char> str) throws(NotEnoughSpaceException);
		void nextLine() throws(NotEnoughSpaceException);

	private:
		io::VOStream<char> m_stream;
		bool m_prev;
	};

}

template <typename Derived, typename C, typename _Info>
void kr::CSVParser::next(OutStream<Derived, C, _Info>* dest) throws(EofException, NoLineException, NextLineException)
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
		char chr = m_stream.peek();
		if (chr == '\"')
		{
			m_stream.skip(1);

			for (;;)
			{
				try
				{
					size_t pos = m_stream.pos('\"');
					if (m_stream.peekAt(pos + 1) != '\"')
					{
						m_stream.read(pos).writeTo(dest);
						m_stream.skip(1);
						break;
					}
					else
					{
						m_stream.read(pos + 1).writeTo(dest);
						m_stream.skip(1);
					}
				}
				catch (TooBigException&)
				{
					m_stream.text().writeTo(dest);
					m_stream.clearBuffer();
				}
			}
		}

		m_stream.readwith_y(",\n", (char*)&finded).writeTo(dest);
	}
	catch (EofException&)
	{
		m_stream.text().writeTo(dest);
		m_stream.clearBuffer();
		finded = eof;
	}

	m_nLast = finded;
}

template <typename Converter, typename C2>
void kr::CSVWriter::write(View<C2> str) throws(NotEnoughSpaceException)
{
	static const auto filter = meta::literal_as<C2>("\"\'\n\r,");
	static const auto from = meta::literal_as<C2>("\"");
	static const auto to = meta::literal_as<char>("\"\"");

	if (m_prev) m_stream.write((char)',');

	bool quote = (str.find_y(filter) != nullptr);
	if (quote) m_stream.write((char)'\"');
	TmpArray<char> temp;
	str.template replace<Converter>(&temp, (View<C2>)from, (View<char>)to);
	m_stream.write(temp);
	if (quote) m_stream.write((char)'\"');
	m_prev = true;
}
