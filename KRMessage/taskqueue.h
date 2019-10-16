#pragma once

#include <KR3/util/callable.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/data/linkedlist.h>

namespace kr
{
	namespace _pri_
	{
		class TaskImpl :public Callable
		{
		public:
			virtual void cancel() noexcept;
		};
	}
	using Task = Node<_pri_::TaskImpl>;

	class TaskQueue
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
		template <typename LAMBDA>
		void post(LAMBDA lambda);
		template <typename LAMBDA, typename CANCEL>
		void post(LAMBDA lambda, CANCEL oncancel);

	private:

		std::atomic<uint> m_leftWorks;
		CriticalSection m_cs;
		Chain<Task> m_works;
		Cond m_event;
	};


	template <typename LAMBDA>
	void TaskQueue::post(LAMBDA lambda)
	{
		struct Data :Task
		{
			LAMBDA m_lambda;

			Data(LAMBDA lamb)
				:m_lambda(move(lamb))
			{
			}
			~Data() noexcept override
			{
			}
			void operator ()() override
			{
				m_lambda();
			}
		};
		attach(_new Data(move(lambda)));
	}

	template <typename LAMBDA, typename CANCEL>
	void TaskQueue::post(LAMBDA lambda, CANCEL oncancel)
	{
		struct Data :Task
		{
			LAMBDA m_lambda;
			CANCEL m_oncancel;

			Data(LAMBDA lamb, CANCEL oncancel)
				:m_lambda(move(lamb)), m_oncancel(move(oncancel))
			{
			}
			~Data() noexcept override
			{
			}
			void operator ()() override
			{
				m_lambda();
			}
			void cancel() noexcept override
			{
				m_oncancel();
			}
		};
		attach(_new Data(move(lambda), move(oncancel)));
	}

}
