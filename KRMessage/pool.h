#pragma once

#include <KR3/main.h>
#include <atomic>
#include <KR3/data/linkedlist.h>
#include <KR3/util/callable.h>
#include <KR3/mt/thread.h>
#include <KR3/mt/criticalsection.h>
#include "poster.h"

#include <KRMessage/promise.h>

namespace kr
{
	class ThreadPool;
	class ThreadTaskBase:public Callable
	{
	public:
		virtual void cancel() noexcept;
	};
	using ThreadTask = Node<ThreadTaskBase>;

	class ThreadPool
	{
	public:
		ThreadPool(int threadCount) noexcept;
		ThreadPool() noexcept;
		~ThreadPool() noexcept;
		ThreadPool(const ThreadPool&)=delete;
		ThreadPool& operator =(const ThreadPool&) = delete;
		void post(ThreadTask * work) noexcept;
		void terminate() noexcept;
		template <typename LAMBDA>
		void postL(LAMBDA lambda);
		template <typename LAMBDA, typename CANCEL>
		void postL(LAMBDA lambda, CANCEL oncancel);
		static ThreadPool * getInstance() noexcept;


	private:
		int _thread() noexcept;

		Array<ThreadObject> m_threads;
		Chain<Node<ThreadTaskBase>> m_works;
		Cond m_event;
		CriticalSection m_cs;
		
		std::atomic<uint> m_leftWorks;
		std::atomic<bool> m_dead;
	};

	template <typename LAMBDA>
	void ThreadPool::postL(LAMBDA lambda)
	{
		struct Data :ThreadTask
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
		post(_new Data(move(lambda)));
	}

	template <typename LAMBDA, typename CANCEL>
	void ThreadPool::postL(LAMBDA lambda, CANCEL oncancel)
	{
		struct Data :ThreadTask
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
		post(_new Data(move(lambda), move(oncancel)));
	}

	template <typename LAMBDA>
	void threadingVoid(LAMBDA lambda) noexcept
	{
		ThreadPool::getInstance()->postL(move(lambda));
	}
	template <typename LAMBDA, typename CANCEL>
	void threadingVoid(LAMBDA lambda, CANCEL oncancel) noexcept
	{
		ThreadPool::getInstance()->postL(move(lambda), move(oncancel));
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
				m_target->postL([this](void*) {
					_resolveCommit();
				});
			}
			void reject() noexcept
			{
				new(_rejectValue()) std::exception_ptr(std::current_exception());
				m_target->postL([this](void*) {
					_rejectCommit();
				});
			}
			void quit() noexcept
			{
				new(_rejectValue()) std::exception_ptr(make_exception_ptr(ThrowAbort()));
				m_target->postL([this](void*) {
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