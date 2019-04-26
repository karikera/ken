#pragma once

#include <stddef.h>

namespace kr
{
	class LLDebugOutput
	{
	public:
		LLDebugOutput() noexcept;
		~LLDebugOutput() noexcept;

		size_t write(const char * src, size_t len) noexcept;
		void putSourceLine(const char * src, int line) noexcept;
		void flush() noexcept;

		LLDebugOutput& operator << (char chr) noexcept;
		LLDebugOutput& operator << (const char * src) noexcept;
		LLDebugOutput& operator << (int n) noexcept;
		template <size_t sz>
		LLDebugOutput& operator << (const char(*str)[sz]) noexcept
		{
			write(str, sz);
			return *this;
		}

	private:
#ifdef WIN32
		char * _destend() noexcept;
		
		static constexpr size_t BUFFERSIZE = 1024;
		char m_temp[BUFFERSIZE+ 1];
		char * m_dest;

#endif
	};
}