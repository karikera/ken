#pragma once

#include <KR3/main.h>

#include "time.h"

namespace kr
{
	/*
	시간 간격을 계산해주는 클래스
	*/
	class DeltaMeasure
	{
	public:
		DeltaMeasure() noexcept;
		float measureDelta() noexcept;

	private:
		timepoint_detail m_time;

	};

}
