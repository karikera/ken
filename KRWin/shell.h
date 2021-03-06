#pragma once

#include <KR3/main.h>
#include <KR3/win/windows.h>
#include <ShellAPI.h>

namespace kr
{
	namespace win
	{
		class DragFileList:public MakeIndexIterable<DragFileList, Text16>
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