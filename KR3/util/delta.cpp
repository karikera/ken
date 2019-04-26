#include "stdafx.h"
#include "delta.h"

kr::DeltaMeasure::DeltaMeasure() noexcept
{
	m_time = timepoint_detail::now();
}

float kr::DeltaMeasure::measureDelta() noexcept
{
	timepoint_detail now = timepoint_detail::now();
	duration_detail dura = now - m_time;
	m_time = now;
	return dura.getRealTime();
}
