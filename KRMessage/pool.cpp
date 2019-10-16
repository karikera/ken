#include "stdafx.h"
#include "pool.h"

#include <KR3/mt/thread.h>
#include <KR3/util/dump.h>

kr::ThreadPool::ThreadPool(int cpuCount) noexcept
{
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
void kr::ThreadPool::terminate() noexcept
{
	struct QuitWork :Task
	{
		void operator ()() override
		{
			throw QuitException(0);
		}
	};
	clearTask();

	for (ThreadObject thread : m_threads)
	{
		QuitWork * work = _new QuitWork;
		attach(work);
	}
	for (ThreadObject thread : m_threads)
	{
		thread.join();
	}
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
				process();
				wait();
			}
		}
		catch (QuitException & quit)
		{
			wake();
			return quit.exitCode;
		}
	});
}
