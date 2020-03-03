#include "stdafx.h"
#include "poster.h"

#ifdef WIN32
#include "pump.h"
#endif

using namespace kr;

Posted::Posted() noexcept
#ifdef WIN32
	:TimerEvent(timepoint())
#endif
{
#ifdef __EMSCRIPTEN__
	m_workerId = 0;
#endif
}

PostTarget * PostTarget::getCurrent() noexcept
{
#ifdef WIN32
	return (PostTarget*)EventPump::getInstance();
#elif defined(__EMSCRIPTEN__)
	return nullptr;
#endif
}
bool PostTarget::attach(Posted * data) noexcept
{
#ifdef WIN32
	if (data->isPosted()) return false;
	data->m_at = timepoint::now();
	((EventPump*)this)->attach(data);
	return true;
#elif defined(__EMSCRIPTEN__)
	return false;
#endif
}
