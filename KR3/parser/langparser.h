#pragma once

#include <KR3/main.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/io/bufferedstream.h>
#include <set>

namespace kr
{
	class LangParser
	{
	public:
		struct ParserException :Exception {};
		struct UnclosedCommentException :ParserException {};
		struct UnexpectedCharacterException :ParserException {
			const char actually;
			char respectedBuffer[4];
			const Text respected;

			UnexpectedCharacterException(char actually, char repected) noexcept;
			UnexpectedCharacterException(char actually, Text repected) noexcept;
		};

		static void test(char actually, Text respected) throws(UnexpectedCharacterException);
		static void test(char actually, char respected) throws(UnexpectedCharacterException);
		static const char DEFAULT_WORD_SPLITTER[22];

		LangParser() noexcept;
		LangParser(Text wordSplitters) noexcept;

		void setWordSplitters(Text wordSplitters) noexcept;
		bool isSplitter(char chr) noexcept;
		uint getLine() noexcept;
		bool eof() noexcept;

		void setStream(io::VIStream<char> is) noexcept;

		char get();
		char peek();

		char skipLine();
		char skipComments();
		char skipSpace();
		void skipSpace(char chr);
		char readWord(TText * out);
		char readWord(AText * out);
		AText readWord(char endCode);
		int readNumber(char endCode);
		
		char skipSpace(Text mask);
		char readWord(AText * out, Text mask);

		template <typename LAMBDA> 
		char readWord(LAMBDA lambda, Text mask)
		{
			AText out;
			char chr = readWord(&out);
			test(chr, mask);
			lambda(out);
			return chr;
		}

	private:
		io::SelfBufferedIStream<io::VIStream<char>, true> m_is;
		std::set<char> m_splitters;
		uint m_line;
		bool m_eof;
	};

	class LangWriter: public OutStream<LangWriter, char, StreamInfo<> >
	{
	public:
		LangWriter();
		void setStream(io::VOStream<char> os) noexcept;
		void indent(int n) noexcept;
		void flush() noexcept;
		void writeImpl(const char * data, size_t size);

	private:
		void _writeText(Text text) noexcept;
		io::BufferedOStream<io::VOStream<char>, false, true> m_os;
		size_t m_indent;
		bool m_endl;
	};
}
