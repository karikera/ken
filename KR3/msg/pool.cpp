#include "stdafx.h"
#include "pool.h"

#include <KR3/mt/thread.h>
#include <KR3/util/dump.h>

using namespace kr;

struct QuitWork :Task
{
	void operator ()() override
	{
		throw QuitException(0);
	}
};

namespace
{
	int taskThread(TaskQueue* queue) noexcept
	{
		return dump_wrap([queue] {
			try
			{
				for (;;)
				{
					queue->process();
					queue->wait();
				}
			}
			catch (QuitException & quit)
			{
				return quit.exitCode;
			}
			});
	}
}

TaskThread::TaskThread() noexcept
{
	m_quited = false;
	m_thread.create<TaskQueue, taskThread>(this);
	ondebug(m_thread.setName("KEN TaskThread"));
}
TaskThread::~TaskThread() noexcept
{
	postQuit();
	m_thread.join();
}
ThreadObject TaskThread::getThreadObject() noexcept
{
	return m_thread;
}
void TaskThread::postQuit() noexcept
{
	if (m_quited) return;
	attach(_new QuitWork);
	m_quited = true;
}
void TaskThread::attach(Task* task) noexcept
{
	_assert(!m_quited); // 작업은 전부 처리하는 것을 목표로 한다
	TaskQueue::attach(task);
}

ThreadPoolKrImpl::ThreadPoolKrImpl(int cpuCount) noexcept
{
	m_threads.resize(cpuCount);

	uint number = 0;
	for (ThreadObject & thread : m_threads)
	{
		thread.create<TaskQueue, &taskThread>(this);
		ondebug(thread.setName(TSZ() << "KEN ThreadPoolKrImpl " << decf(number++, 2)));
	}
}
ThreadPoolKrImpl::ThreadPoolKrImpl() noexcept
	:ThreadPoolKrImpl(getCPUCount())
{
}
ThreadPoolKrImpl::~ThreadPoolKrImpl() noexcept
{
	for (ThreadObject thread : m_threads)
	{
		attach(_new QuitWork);
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

ThreadPoolWinImpl* ThreadPoolWinImpl::getInstance() noexcept
{
	static ThreadPoolWinImpl instance;
	return &instance;
}
void ThreadPoolWinImpl::attach(Task* work) noexcept
{
	if (!QueueUserWorkItem([](void* context) {
			Must<Task> work = (Task*)context;
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
