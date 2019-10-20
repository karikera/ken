#pragma once

#include <KR3/main.h>
#include <KR3/data/linkedlist.h>

namespace kr
{
	using Logger = Node<io::VOStream<char>>;
	class LogManager:public LinkedList<Logger>
	{
	public:
		void write(kr::Text text) noexcept;
		void printf(pcstr agent, pcstr format, ...) noexcept;
		void puts(pcstr agent, pcstr str) noexcept;
		void putch(char chr) noexcept;

		Logger* pushStdOut() noexcept;
		Logger* pushErrorOut() noexcept;

	private:
	};

	extern LogManager g_log;
}