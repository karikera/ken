#pragma once

#include <KR3/main.h>
#include <KR3/wl/windows.h>
#include <KR3/data/iterator.h>
#include <ShellAPI.h>

namespace kr
{
	namespace win
	{


		class DragFileList:public TIndexIterable<DragFileList>
		{
		public:
			DragFileList(HDROP drop) noexcept;
			DragFileList(WPARAM drop) noexcept;
			~DragFileList() noexcept;

			uint size() noexcept;
			Text16 operator [](uint i) noexcept;

		private:
			char16 m_fileName[MAX_PATH];
			uint m_count;
			HDROP m_drop;
		};

	}
}