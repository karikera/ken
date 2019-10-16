#pragma once
#include <KR3/main.h>
#include <atomic>
#include <KR3/mt/thread.h>
#include "poster.h"

#include <KRMessage/promise.h>
#include "taskqueue.h"

namespace kr
{
	class ThreadPool;

	class ThreadPool:public TaskQueue
	{
	public:
		ThreadPool(int threadCount) noexcept;
		ThreadPool() noexcept;
		~ThreadPool() noexcept;
		void terminate() noexcept;
		static ThreadPool * getInstance() noexcept;

	private:
		int _thread() noexcept;

		Array<ThreadObject> m_threads;
		std::atomic<bool> m_dead;
	};

	template <typename LAMBDA>
	void threadingVoid(LAMBDA lambda) noexcept
	{
		ThreadPool::getInstance()->post(move(lambda));
	}
	template <typename LAMBDA, typename CANCEL>
	void threadingVoid(LAMBDA lambda, CANCEL oncancel) noexcept
	{
		ThreadPool::getInstance()->post(move(lambda), move(oncancel));
	}
	template <typename LAMBDA>
	auto threading(LAMBDA lambda) noexcept ->Promise<decltype(lambda())> *
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
		threadingVoid([prom, lambda = move(lambda)]() mutable {
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