#pragma once

#include <KR3/mt/criticalsection.h>

#include "task.h"

namespace kr
{
	class TaskQueue:public TaskLambdaPost<TaskQueue>
	{
	public:
		void process() noexcept;
		void wait() noexcept;
		void wake() noexcept;

		TaskQueue() noexcept;
		TaskQueue(const TaskQueue&) = delete;
		TaskQueue& operator =(const TaskQueue&) = delete;
		void clearTask() noexcept;
		void attach(Task* work) noexcept;

	private:

		std::atomic<uint> m_leftWorks;
		CriticalSection m_cs;
		Chain<Task> m_works;
		Cond m_event;
	};

}
