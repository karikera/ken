#pragma once

#include <KR3/mt/criticalsection.h>

#include "task.h"

namespace kr
{
	class TaskQueue:public TaskLambdaPost<TaskQueue>
	{
	public:
		// 받은 Task에서 예외를 던지는걸 고려한다
		void process() throws(...);
		void wait() noexcept;
		void wake() noexcept;

		TaskQueue() noexcept;
		TaskQueue(const TaskQueue&) = delete;
		~TaskQueue() noexcept;
		TaskQueue& operator =(const TaskQueue&) = delete;
		ATTR_DEPRECATED("all tasks must complete") void clearTask() noexcept;
		void attach(Task* work) noexcept;

	private:

		std::atomic<uint> m_leftWorks;
		CriticalSection m_cs;
		Chain<Task> m_works;
		Cond m_event;
	};

}
