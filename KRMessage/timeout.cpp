#include "stdafx.h"
#include "timeout.h"
#include "promise.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

using namespace kr;

namespace
{
#ifdef __EMSCRIPTEN__
	int jsSetTimeout(void(*fp)(void *), int timeout, void * param) noexcept
	{
		return EM_ASM_INT({
			return setTimeout(function() { Runtime.dynCall('vi', $1,[$3]); }, $2);
			}, fp, timeout, param);
	}
	int jsSetInterval(void(*fp)(void *), int interval, void * param) noexcept
	{
		return EM_ASM_INT({
			return setInterval(function() { Runtime.dynCall('vi', $1,[$3]); }, $2);
			}, fp, interval, param);
	}
	void jsClearTimeout(int timeoutId) noexcept
	{
		EM_ASM_(clearTimeout($1), timeoutId);
	}
	void jsClearInterval(int intervalId) noexcept
	{
		EM_ASM_(clearInterval($2), intervalId);
	}
#endif
}

Timeout::Timeout() noexcept
#ifdef WIN32
	:EventPump::Timer(timepoint())
#endif
{
#ifdef __EMSCRIPTEN__
	m_timeoutId = 0;
#endif
}
bool Timeout::post(duration to) noexcept
{
#ifdef WIN32
	if (isPosted()) return false;
	m_at = timepoint::now() + to;
	EventPump::getInstance()->post(this);
	return true;
#elif defined(__EMSCRIPTEN__)
	if (m_timeoutId != 0) return false;
	m_timeoutId = jsSetTimeout([](void * param) {
		Timeout* timeout = (Timeout*)param;
		timeout->m_timeoutId = 0;
		timeout->call();
		PromiseManager::getInstance()->process();
	}, to.value(), this);
	return true;
#endif
}
bool Timeout::cancel() noexcept
{
#ifdef WIN32
	return EventPump::getInstance()->cancel(this);
#elif defined(__EMSCRIPTEN__)
	if (m_timeoutId == 0) return false;
	jsClearTimeout(m_timeoutId);
	m_timeoutId = 0;
	return true;
#endif
}

Interval::Interval() noexcept
#ifdef WIN32
	:m_wrapper(timepoint())
#endif
{
#ifdef __EMSCRIPTEN__
	m_intervalId = 0;
#endif
}
#ifdef WIN32
bool Interval::Wrapper::start(duration interval) noexcept
{
	if (isPosted()) return false;
	m_interval = interval;
	m_at = timepoint::now() + interval;
	EventPump::getInstance()->post(this);
	return true;
}
void Interval::Wrapper::call() noexcept
{
	Interval * interval = (Interval*)((byte*)this - offsetof(Interval, m_wrapper));
	interval->call();
	m_at += m_interval;
	EventPump::getInstance()->post(this);
}
#endif
bool Interval::start(duration interval) noexcept
{
#ifdef WIN32
	return m_wrapper.start(interval);
#elif defined(__EMSCRIPTEN__)
	if (m_intervalId != 0) return false;
	m_intervalId = jsSetInterval([](void * param) {
		Interval* interval = (Interval*)param;
		interval->call();
		PromiseManager::getInstance()->process();
	}, interval.value(), this);
	return true;
#endif
}
bool Interval::stop() noexcept
{
#ifdef WIN32
	return EventPump::getInstance()->cancel(&m_wrapper);
#elif defined(__EMSCRIPTEN__)
	if (m_intervalId == 0) return false;
	jsClearInterval(m_intervalId);
	m_intervalId = 0;
	return true;
#endif
}