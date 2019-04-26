#pragma once

#include "../main.h"
#include "../meta/chreturn.h"

#ifdef WIN32
#include "../wl/threadhandle.h"
#else
#include <pthread.h>
#endif

namespace kr
{
	template <typename LAMBDA> class LambdaThread;

	class ThreadObject
	{
	private:
#ifdef WIN32
		ThreadHandle * m_handle;
#else
		pthread_t m_handle;
#endif

	public:
#ifdef WIN32
		using RawThreadReturn = unsigned long;
		using RawThreadProc = RawThreadReturn(CT_STDCALL *)(void*);
#else
		using RawThreadReturn = void*;
		using RawThreadProc = RawThreadReturn(*)(void*);
#endif
		using RawThreadReturnInt = int_sz_t<sizeof(RawThreadReturn)>;

		static ThreadObject getCurrent() noexcept;
		void createRaw(RawThreadProc fn, void * param
#ifdef WIN32
			,unsigned long * id = nullptr
#endif
		) noexcept;

		template <typename T, int(*fn)(T*)>
		void create(T * param) noexcept
		{
			return createRaw([](void * param){ return (RawThreadReturn)(RawThreadReturnInt)*fn((T*)param); }, param);
		}
		template <typename This, int (This::*fn)()>
		void create(This * param) noexcept
		{
			return createRaw([](void * param){ return (RawThreadReturn)(RawThreadReturnInt)(((This*)param)->*fn)(); }, param);
		}
		template <typename This, int (This::*fn)() const>
		void create(This * param) noexcept
		{
			return createRaw([](void*param){ return (RawThreadReturn)(RawThreadReturnInt)((This*)param)->*fn(); }, param);
		}

		template <typename LAMBDA>
		void createLambda(LAMBDA lambda)
		{
			using NLAMBDA = meta::ChReturn<int, LAMBDA>;
			NLAMBDA * plambda = _new NLAMBDA(move(lambda));
			return createRaw([](void * p){ 
				int ret = (int)(*(NLAMBDA*)p)();
				delete (NLAMBDA*)p;
				return (RawThreadReturn)(RawThreadReturnInt)ret;
			}, plambda);
		}

		ThreadObject() noexcept = default;
		ThreadObject(const ThreadObject &) noexcept = default;
		ThreadObject(nullptr_t) noexcept
		{
#ifdef WIN32
			m_handle = nullptr;
#else
			m_handle = 0;
#endif
		}
		bool operator ==(const ThreadObject & other) const noexcept
		{
			return m_handle == other.m_handle;
		}
		bool operator !=(const ThreadObject & other) const noexcept
		{
			return m_handle != other.m_handle;
		}
		bool operator ==(nullptr_t) const noexcept
		{
#ifdef WIN32
			return m_handle == nullptr;
#else
			return m_handle == 0;
#endif
		}
		bool operator !=(nullptr_t) const noexcept
		{
#ifdef WIN32
			return m_handle != nullptr;
#else
			return m_handle != 0;
#endif
		}
		friend bool operator ==(nullptr_t, const ThreadObject & _this) noexcept
		{
#ifdef WIN32
			return _this.m_handle == nullptr;
#else
			return _this.m_handle == 0;
#endif
		}
		friend bool operator !=(nullptr_t, const ThreadObject & _this) noexcept
		{
#ifdef WIN32
			return _this.m_handle != nullptr;
#else
			return _this.m_handle != 0;
#endif
		}

		int join() noexcept
		{
#ifdef WIN32
			return (int)m_handle->join();
#else
			void * state;
			pthread_join(m_handle, &state);
			return (int)(intptr_t)state;
#endif
		}
		void terminate() noexcept
		{
#ifdef WIN32
			m_handle->terminate();
#else
			pthread_cancel(m_handle);
#endif
		}
		void setName(pcstr name) noexcept
		{
#ifdef WIN32
			m_handle->getId().setName(name);
#elif __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 12
			pthread_setname_np(m_handle, name);
#endif
		}
	};

	template <typename T> class Threadable
	{
	private:
		ThreadObject m_obj;

	public:
		inline Threadable() noexcept
		{
			m_obj = nullptr;
		}
		inline ~Threadable() noexcept
		{
		}
		inline void start() noexcept
		{
			m_obj.create<T, &T::thread>(static_cast<T*>(this));
		}
		inline void terminate() noexcept
		{
			if (m_obj != nullptr)
			{
				m_obj.terminate();
				m_obj = nullptr;
			}
		}

		inline bool isCurrentThread() const noexcept
		{
			return ThreadObject::getCurrent() == m_obj;
		}
		inline ThreadObject getThreadObject() const noexcept
		{
			return m_obj;
		}

		inline void join() noexcept
		{
			m_obj.join();
			m_obj = nullptr;
		}
		inline bool exists() noexcept
		{
			return m_obj != nullptr;
		}
	};

	class Thread:public Threadable<Thread>
	{
	public:
		Thread() noexcept;
		virtual ~Thread() noexcept;
		virtual int thread() noexcept = 0;

		template <typename LAMBDA>
		static LambdaThread<LAMBDA>* wrap(LAMBDA lambda) noexcept;
	};
	template <typename LAMBDA> class LambdaThread:public Thread
	{
	public:
		LambdaThread(LAMBDA lambda);
		~LambdaThread() override;
		int thread() noexcept override;
	private:
		LAMBDA m_lambda;
	};
}

template <typename LAMBDA>
kr::LambdaThread<LAMBDA>* kr::Thread::wrap(LAMBDA lambda) noexcept
{
	return _new LambdaThread<LAMBDA>(move(lambda));
}
template <typename LAMBDA>
kr::LambdaThread<LAMBDA>::LambdaThread(LAMBDA lambda)
	:m_lambda(move(lambda))
{
}
template <typename LAMBDA>
kr::LambdaThread<LAMBDA>::~LambdaThread()
{
}
template <typename LAMBDA>
int kr::LambdaThread<LAMBDA>::thread() noexcept
{
	return meta::chreturn<int>(m_lambda)();
}