#include "stdafx.h"
#include "taskqueue.h"

using namespace kr;

TaskQueue::TaskQueue() noexcept
	:m_leftWorks(0)
{
}
TaskQueue::~TaskQueue() noexcept
{
	_assert(m_works.empty()); // 작업은 전부 처리하는 것을 목표로 한다
}
void TaskQueue::process() throws(...)
{
	for (;;)
	{
		if (m_leftWorks == 0) return;
		Task* work;
		{
			CsLock lock(m_cs);
			if (m_works.empty()) continue;
			work = m_works.detachFirst();
		}
		m_leftWorks--;
		Must<Task> kill = work;
		work->call();
	}
}
void TaskQueue::wait() noexcept
{
	m_event.wait();
}
void TaskQueue::wake() noexcept
{
	m_event.set();
}

void TaskQueue::clearTask() noexcept
{
	CsLock lock(m_cs);
	for (Task& work : m_works)
	{
		work.cancel();
	}
	m_works.clear();
}
void TaskQueue::attach(Task* work) noexcept
{
	m_leftWorks++;
	CsLock lock(m_cs);
	m_works.attach(work);
	m_event.set();
}