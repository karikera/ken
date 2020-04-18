#pragma once
#include <KR3/main.h>
#include <atomic>
#include <KR3/mt/thread.h>
#include "poster.h"

#include <KR3/msg/promise.h>
#include <KR3/mt/taskqueue.h>

namespace kr
{
	class ThreadPoolKrImpl;

	class TaskThread :public TaskLambdaPost<TaskThread, Protected<TaskQueue> >
	{
	public:
		TaskThread() noexcept;
		~TaskThread() noexcept;
		ThreadObject getThreadObject() noexcept;
		void postQuit() noexcept;
		using TaskLambdaPost::post;
		void attach(Task* task) noexcept;

	private:
		ThreadObject m_thread;
		bool m_quited;
	};

	class ThreadPoolKrImpl:private TaskQueue
	{
	public:
		static ThreadPoolKrImpl * getInstance() noexcept;
		using TaskQueue::post;
		using TaskQueue::attach;

	private:
		ThreadPoolKrImpl(int threadCount) noexcept;
		ThreadPoolKrImpl() noexcept;
		~ThreadPoolKrImpl() noexcept;

		Array<ThreadObject> m_threads;
	};

#ifdef WIN32

	class ThreadPoolWinImpl:public TaskLambdaPost<ThreadPoolWinImpl>
	{
	public:
		static ThreadPoolWinImpl* getInstance() noexcept;
		void attach(Task* work) noexcept;

	private:
		ThreadPoolWinImpl() noexcept;
		~ThreadPoolWinImpl() noexcept;

	};

	using ThreadPool = ThreadPoolWinImpl;
#else
	using ThreadPool = ThreadPoolKrImpl;
#endif

	template <typename LAMBDA>
	void threadingVoid(LAMBDA &&lambda) noexcept
	{
		ThreadPool::getInstance()->post(forward<LAMBDA>(lambda));
	}
	template <typename LAMBDA, typename CANCEL>
	void threadingVoid(LAMBDA &&lambda, CANCEL &&oncancel) noexcept
	{
		ThreadPool::getInstance()->post(forward<LAMBDA>(lambda), forward<CANCEL>(oncancel));
	}
	template <typename LAMBDA>
	auto threading(LAMBDA &&lambda) noexcept ->Promise<decltype(lambda())> *
	{
		using T = decltype(lambda());

		class PromiseImpl: public Promise<T>
		{
		private:
			PostTarget * m_target;
			using Promise<T>::_resolveCommit;
			using Promise<T>::_rejectCommit;

		public:
			using Promise<T>::_resolveValue;
			using Promise<T>::_rejectValue;

			PromiseImpl() noexcept
			{
				m_target = PostTarget::getCurrent();
			}

			void resolve() noexcept
			{
				m_target->post([this](void*) {
					_resolveCommit();
				});
			}
			void reject() noexcept
			{
				new(_rejectValue()) std::exception_ptr(std::current_exception());
				m_target->post([this](void*) {
					_rejectCommit();
				});
			}
			void quit() noexcept
			{
				new(_rejectValue()) std::exception_ptr(make_exception_ptr(ThrowAbort()));
				m_target->post([this](void*) {
					_rejectCommit();
				});
			}
		};

		PromiseImpl * prom = _new PromiseImpl;
		threadingVoid([prom, lambda = forward<LAMBDA>(lambda)]() mutable {
			try
			{
				promise_meta::with<void, T>::call(prom->_resolveValue(), lambda, nullptr);
				prom->resolve();
			}
			catch (...)
			{
				prom->reject();
			}
		}, [prom] {
			prom->quit();
		});
		return prom;
	}
}