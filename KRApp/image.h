#pragma once

#include <KR3/main.h>
#include <KR3/data/idmap.h>

namespace kr
{
	class WebVariable
	{
	public:
		WebVariable() noexcept;
		~WebVariable() noexcept;

	protected:
		void _allocateIndex() noexcept;
		void _freeIndex() noexcept;
		int m_index;

	private:
		static int s_lastIndex;
		static SortedArray<int> s_freed;

	};
	class WebImage
	{
	public:
		WebImage() = delete;

		static WebImage * load(const char * name);
		static void operator delete(void * ptr) noexcept;

	};
}

