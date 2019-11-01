#include "stdafx.h"

#ifdef WIN32
#include "progressor.h"
#include "pump.h"
#include <KR3/mt/thread.h>

kr::ProgressNotifier::ProgressNotifier() noexcept
{
	m_length = 0;
	m_progress = 0;
	m_start = timepoint::now();
}
kr::ProgressNotifier::~ProgressNotifier() noexcept
{
}

void kr::ProgressNotifier::set(int64_t pr) noexcept
{
	timepoint now = timepoint::now();
	if (now - m_start < (duration)200) return;
	if (m_progress == pr) return;
	m_progress = pr;
	onProgress();
	m_start = now;
}
void kr::ProgressNotifier::set(int64_t pr, int64_t length) noexcept
{
	m_length = length;
	set(pr);
}
void kr::ProgressNotifier::setEnd() noexcept
{
	m_progress = m_length;
	onProgress();
}
int64_t kr::ProgressNotifier::progress() noexcept
{
	return m_progress;
}
int64_t kr::ProgressNotifier::length() noexcept
{
	return m_length;
}

kr::Progressor::Progressor() noexcept
{
	m_thread = nullptr;
}
kr::Progressor::~Progressor() noexcept
{
	_assert(m_thread == nullptr);
}
bool kr::Progressor::closed() noexcept
{
	if(m_thread == nullptr) return true;
	return false;
}
void kr::Progressor::start() noexcept
{
	_assert(m_thread == nullptr);
	m_readyEvent = EventHandle::create(false, false);
	ThreadHandle * thread = ThreadHandle::createRaw(_thread, this, &m_threadId);
	EventPump::getInstance()->wait(m_readyEvent);
	delete m_readyEvent;
	m_thread = thread;
}
void kr::Progressor::quit(int exitCode) noexcept
{
	_assert(!closed());
	bool postQuitMessage = m_threadId.quit(exitCode);
	_assert(postQuitMessage);
}
void kr::Progressor::join() noexcept
{
	_assert(m_thread != nullptr);
	m_thread->join();
	m_thread = nullptr;
}
int64_t kr::Progressor::getProgress() noexcept
{
	return m_progress;
}
int64_t kr::Progressor::getLength() noexcept
{
	return m_length;
}
void kr::Progressor::setProgress(int64_t progress) noexcept
{
	set(progress);
}
void kr::Progressor::setProgress(int64_t progress, int64_t length) noexcept
{
	set(progress, length);
}
void kr::Progressor::checkPoint() throws(QuitException)
{
	EventPump::getInstance()->processOnce();
}
void kr::Progressor::checkPoint(long long nProgress) // ThrowAbort
{
	set(nProgress);
	checkPoint();
}
int kr::Progressor::_thread(Progressor * _this) noexcept
{
	{
		MSG msg;
		PeekMessageW(&msg, nullptr, 0, 0, 0);
	}
	_this->m_readyEvent->set();
	return _this->progressor();
}

#else

EMPTY_SOURCE

#endif
