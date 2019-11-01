#include "stdafx.h"
#include "pool.h"

#include <KR3/mt/thread.h>
#include <KR3/util/dump.h>

using namespace kr;

ThreadPoolKrImpl::ThreadPoolKrImpl(int cpuCount) noexcept
{
	m_threads.resize(cpuCount);

	uint number = 0;
	for (ThreadObject & thread : m_threads)
	{
		thread.create<ThreadPoolKrImpl, &ThreadPoolKrImpl::_thread>(this);
		ondebug(thread.setName(TSZ() << "KEN ThreadPoolKrImpl " << decf(number++, 2)));
	}
	// QueueUserWorkItem();
}
ThreadPoolKrImpl::ThreadPoolKrImpl() noexcept
	:ThreadPoolKrImpl(getCPUCount())
{
}
ThreadPoolKrImpl::~ThreadPoolKrImpl() noexcept
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
		QuitWork* work = _new QuitWork;
		attach(work);
	}
	for (ThreadObject thread : m_threads)
	{
		thread.join();
	}
	m_threads = nullptr;
}
ThreadPoolKrImpl * ThreadPoolKrImpl::getInstance() noexcept
{
	static ThreadPoolKrImpl instance;
	return &instance;
}
int ThreadPoolKrImpl::_thread() noexcept
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

ThreadPoolWinImpl* ThreadPoolWinImpl::getInstance() noexcept
{
	static ThreadPoolWinImpl instance;
	return &instance;
}
void ThreadPoolWinImpl::attach(Task* work) noexcept
{
	if (!QueueUserWorkItem([](void* context) {
			auto* work = (Task*)context;
			work->call();
			return (DWORD)0;
		}, work, 0))
	{
		work->cancel();
	}
}

ThreadPoolWinImpl::ThreadPoolWinImpl() noexcept
{
}
ThreadPoolWinImpl::~ThreadPoolWinImpl() noexcept
{
}
