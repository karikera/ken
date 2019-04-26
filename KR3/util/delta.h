#pragma once

#include <KR3/main.h>

#include "time.h"

namespace kr
{
	/*
	�ð� ������ ������ִ� Ŭ����
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
