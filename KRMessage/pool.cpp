#include "stdafx.h"
#include "pool.h"

#include <KR3/mt/thread.h>
#include <KRUtil/dump.h>

void kr::ThreadTaskBase::cancel() noexcept
{
}

kr::ThreadPool::ThreadPool(int cpuCount) noexcept
{
	m_leftWorks = 0;
	m_threads.resize(cpuCount);

	uint number = 0;
	for (ThreadObject & thread : m_threads)
	{
		thread.create<ThreadPool, &ThreadPool::_thread>(this);
		ondebug(thread.setName(TSZ() << "KEN ThreadPool " << decf(number++, 2)));
	}
}
kr::ThreadPool::ThreadPool() noexcept
	:ThreadPool(getCPUCount())
{
}
kr::ThreadPool::~ThreadPool() noexcept
{
	terminate();
}
void kr::ThreadPool::post(ThreadTask * work) noexcept
{
	m_leftWorks++;
	CsLock lock(m_cs);
	m_works.attach(work);
	m_event.set();
}
void kr::ThreadPool::terminate() noexcept
{
	struct QuitWork :ThreadTask
	{
		void operator ()() override
		{
			throw QuitException(0);
		}
	};
	for (ThreadTask & work : m_works)
	{
		work.cancel();
	}
	m_works.clear();

	for (ThreadObject thread : m_threads)
	{
		QuitWork * work = _new QuitWork;
		m_leftWorks++;
		CsLock lock(m_cs);
		m_works.attach(work);
		m_event.set();
	}
	for (ThreadObject thread : m_threads)
	{
		thread.join();
	}
	m_works.clear();
	m_threads = nullptr;
}
kr::ThreadPool * kr::ThreadPool::getInstance() noexcept
{
	static ThreadPool instance;
	return &instance;
}
int kr::ThreadPool::_thread() noexcept
{
	return dump_wrap([this] {
		try
		{
			for (;;)
			{
				if (m_leftWorks != 0)
				{
					ThreadTask * work;
					{
						CsLock lock(m_cs);
						if (m_works.empty()) continue;
						work = m_works.detachFirst();
					}
					m_leftWorks--;
					Must<ThreadTask> kill = work;
					work->call();
				}
				else
				{
					m_event.wait();
				}
			}
		}
		catch (QuitException & quit)
		{
			m_event.set();
			return quit.exitCode;
		}
	});
}
