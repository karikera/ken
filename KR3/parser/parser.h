#pragma once

#include <KR3/main.h>
#include <KR3/io/bufferedstream.h>
#include "../text/slash.h"

namespace kr
{
	class Parser
	{
	public:
		Parser(io::VIStream<char> is) noexcept;

		bool isNextWhiteSpace() throws(EofException);
		void skipWhiteSpace() throws(...);
		void skipToY(Text needle) throws(EofException);
		void skipTo(Text needle) throws(EofException);
		void skipTo(char needle) throws(EofException);
		void skipToLine() throws(EofException);
		void mustWhiteSpace() throws(InvalidSourceException);
		size_t getLine() noexcept;
		void skip(size_t sz) throws(EofException);
		template <typename _Derived, typename _Info>
		size_t readToSpace(OutStream<_Derived, char, _Info> * os) throws(EofException);
		TSZ readToSpace() throws(EofException);
		template <typename _Derived, typename _Info>
		size_t readToLine(OutStream<_Derived, char, _Info>* os) throws(EofException);
		TSZ readToLine() throws(EofException);

		template <typename _Derived, typename _Info>
		void readWithUnslash(OutStream<_Derived, char, _Info> * dest, char needle) throws(EofException)
		{
			for (;;)
			{
				m_is.readto_L(dest, [&](Text text) {
					const char needles[] = { (char)'\\', needle };
					pcstr finded = text.find_y(Text(needles, countof(needles)));
					if (finded != nullptr)
						m_line += text.cut(finded).count('\n');
					return finded;
				});
				char findedchr = m_is.read();

				if (findedchr == '\\')
				{
					parseUnslashCharacter(dest, &m_is);
				}
				else return;
			}
		}
		TSZ readWithUnslash(char needle) throws(EofException);
		void skipWithUnslash(char needle) throws(EofException);

		io::BufferedIStream<io::VIStream<char>, true> * is() noexcept;
		io::BufferedIStream<io::VIStream<char>, true> * operator->() noexcept;

	protected:
		io::BufferedIStream<io::VIStream<char>, true> m_is;
		size_t m_line;
	};

	template <typename _Derived, typename _Info>
	size_t Parser::readToSpace(OutStream<_Derived, char, _Info> * os) throws(EofException)
	{
		return m_is.readto_y(os, Text::WHITE_SPACE);
	}
	template <typename _Derived, typename _Info>
	size_t Parser::readToLine(OutStream<_Derived, char, _Info>* os) throws(EofException)
	{
		return m_is.readto_y(os, "\r\n");
	}
}