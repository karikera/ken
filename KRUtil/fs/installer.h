#pragma once

namespace kr
{
	class Wildcard
	{
	public:
		Wildcard() noexcept;
		Wildcard(Text16 wildcard) noexcept;
		~Wildcard() noexcept;
		bool test(Text16 text) noexcept;

	private:
		Array<Text16> m_matches;
		Text16 m_firstMatch;
		Text16 m_lastMatch;
		bool m_fullMatch;
	};
	class Installer
	{
	public:
		Installer(Text16 dest, Text16 src, Wildcard wildcard = Wildcard()) noexcept;

		pcstr16 getSrcSz(Text16 name) noexcept;
		pcstr16 getDestSz(Text16 name) noexcept;
		Text16 getSrc(Text16 name) noexcept;
		Text16 getDest(Text16 name) noexcept;
		void all() noexcept;
		void filter(Text16 filterText) noexcept;
		void dir(Text16 dirname) noexcept;
		void file(Text16 filename) noexcept;
		void copy(Text16 dest, Text16 src, Text16 msg) noexcept;

		size_t m_copyCount;
		size_t m_skipCount;
		size_t m_errorCount;

	private:
		AText16 m_src;
		AText16 m_dest;
		size_t m_srcend;
		size_t m_destend;
		Wildcard m_wildcard;
	};
}
