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

		// 프레임이 늦을경우, false를 반환한다.
		bool sync();

	protected:
		uint m_skip;
		duration m_wait;
		timepoint m_next;

	};
}
