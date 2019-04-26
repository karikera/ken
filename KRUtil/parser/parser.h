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

		bool isNextWhiteSpace();
		void skipWhiteSpace();
		void mustWhiteSpace() throws(InvalidSourceException);
		int getLine() noexcept;
		template <typename _Derived, typename _Info>
		size_t readToSpace(OutStream<_Derived, char, _Info> * os) throws(NotEnoughSpaceException);
		TSZ readToSpace();

		template <typename _Derived, typename _Info>
		void readWithUnslash(OutStream<_Derived, char, _Info> * dest, char needle) throws(EofException, NotEnoughSpaceException)
		{
			for (;;)
			{
				m_is.readto_F(dest, [&](Text text) {
					const char needles[] = { (char)'\\', needle };
					Text finded = text.find_y(Text(needles, countof(needles)));
					if (finded != nullptr)
						m_line += (int)text.cut(finded).count('\n');
					return finded.begin();
				});
				char findedchr = m_is.read();

				if (findedchr == '\\')
				{
					parseUnslashCharacter(dest, &m_is);
				}
				else return;
			}
		}
		TSZ readWithUnslash(char needle);
		void skipWithUnslash(char needle);

		io::BufferedIStream<io::VIStream<char>, true> * is() noexcept;
		io::BufferedIStream<io::VIStream<char>, true> * operator->() noexcept;

	protected:
		io::BufferedIStream<io::VIStream<char>, true> m_is;
		int m_line;
	};

	template <typename _Derived, typename _Info>
	size_t Parser::readToSpace(OutStream<_Derived, char, _Info> * os) throws(NotEnoughSpaceException)
	{
		return m_is.readto_y(os, Text::WHITE_SPACE);
	}
}