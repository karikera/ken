#include "stdafx.h"
#include "time.h"

#ifdef WIN32
#include "../wl/windows.h"
#else
#include <time.h>
#endif

using namespace kr;

namespace
{
#ifdef WIN32
	qword queryFreq;
	float queryFreqFloat;

	staticCode
	{
		BOOL QueryPerformanceFrequencyResult = QueryPerformanceFrequency((LARGE_INTEGER*)&queryFreq);
		queryFreqFloat = 1.f / queryFreq;
		_assert(QueryPerformanceFrequencyResult);
	};
#else
	constexpr float queryFreq = 1000000000;
	constexpr float queryFreqFloat = 1 / queryFreq;
#endif
}
template <>
const duration duration::second = (duration)1000;
template <>
const duration_detail duration_detail::second = (duration_detail)queryFreq;

template <>
duration_detail duration_detail::fromRealTime(float realTime) noexcept
{
	return duration_detail((int64_t)((double)queryFreq * realTime));
}
template <>
float duration_detail::getRealTime() const noexcept
{
	return m_time * queryFreqFloat;
}
template <>
duration duration::fromRealTime(float realTime) noexcept
{
	return duration((int32_t)(realTime * 1000.f));
}
template <>
float duration::getRealTime() const noexcept
{
	return m_time * 0.001f;
}
template <>
timepoint timepoint::now() noexcept
{
#ifdef WIN32
	return GetTickCount();
#else
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (qword)ts.tv_sec * 1000 + (qword)ts.tv_nsec / 1000000;
#endif
}
template <>
timepoint_detail timepoint_detail::now() noexcept
{
#ifdef WIN32
	qword tick;
	BOOL QueryPerformanceCounterResult = QueryPerformanceCounter((LARGE_INTEGER*)&tick);
	_assert(QueryPerformanceCounterResult);
	return tick;
#else
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (qword)ts.tv_sec * 1000000000 + (qword)ts.tv_nsec;
#endif
}

TimeChecker::TimeChecker() noexcept
{
}
void TimeChecker::axis() noexcept
{
	timepoint_detail now = timepoint_detail::now();
	m_delta = (now - m_time).getRealTime();
	m_time = now;
}
float TimeChecker::getDelta() noexcept
{
	return m_delta;
}
timepoint_detail TimeChecker::getTime() noexcept
{
	return m_time;
}
void TimeChecker::initTime() noexcept
{
	m_delta = 0.f;
	m_time = timepoint_detail::now();
}
