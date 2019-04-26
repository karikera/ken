#pragma once

#include <KR3/main.h>
#include <KR3/util/time.h>

namespace kr
{
	class FrameSync
	{
	public:
		FrameSync();
		FrameSync(duration framewait);

		// �������� �������, false�� ��ȯ�Ѵ�.
		bool sync();

	protected:
		uint m_skip;
		duration m_wait;
		timepoint m_next;

	};
}
