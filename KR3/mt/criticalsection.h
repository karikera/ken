#pragma once

#include <KR3/main.h>
#include <KR3/data/idmap.h>
#include <KR3/util/time.h>

#ifndef WIN32
#include <semaphore.h>
#endif

namespace kr
{
	class CriticalSection;
	class CsLock
	{
	public:
		CsLock(CriticalSection & cs) noexcept;
		~CsLock() noexcept;

	private:
		CriticalSection * m_cs;
	};

	template <size_t count>
	class CsLocks
	{
	public:
		CsLocks(View<CriticalSection*> css) noexcept;
		~CsLocks() noexcept;

	private:
		BSortedArray<CriticalSection*, count, GetPointerAsKey> m_cs;
	};

	template <>
	class CsLocks<2>
	{
	public:
		CsLocks(CriticalSection& cs1, CriticalSection& cs2) noexcept;
		~CsLocks() noexcept;

	private:
		CriticalSection * m_cs1, *m_cs2;
	};

	class CriticalSection final
	{
		friend CsLock;
	public:
		CriticalSection() noexcept;
		~CriticalSection() noexcept;
		bool tryEnter() noexcept;
		void enter() noexcept;
		void leave() noexcept;

	protected:
#ifdef WIN32

#pragma pack(push, 8)
		struct CRITICAL_SECTION_DATA {
			void * DebugInfo;
			long LockCount;
			long RecursionCount;
			void* OwningThread;        // from the thread's ClientId->UniqueThread
			void* LockSemaphore;
			uintptr_t SpinCount;        // force size on 64-bit systems when packed
		};
#pragma pack(pop)


		CRITICAL_SECTION_DATA m_cs;
#else
		pthread_mutex_t m_mutex;
#endif
	};

#ifdef WIN32
	class EventHandle;
#endif
	
	class Cond
	{
	public:
		Cond() noexcept;
		~Cond() noexcept;

		void set() noexcept;
		void reset() noexcept;
		void wait() noexcept;
		bool wait(duration time) noexcept;

	private:
#ifdef WIN32
		EventHandle * m_handle;
#else
		pthread_cond_t m_cond;
		pthread_mutex_t m_mutex;
#endif
	};

	class RWLock
	{
	public:
		RWLock() noexcept;
		void enterRead() noexcept;
		void leaveRead() noexcept;
		void changeToWrite() noexcept;
		void enterWrite() noexcept;
		void leaveWrite() noexcept;
		void leaveAnyway() noexcept;

	private:
		std::atomic<int> m_reading;
		Cond m_cond;

	};

	class Semaphore
	{
	private:
#ifdef WIN32
		void * m_semaphore;
#else
		sem_t m_semaphore;
#endif

	public:
		Semaphore(int init) noexcept;
		void release(int count) noexcept;
		void wait() noexcept;
	};

	template <size_t count>
	CsLocks<count>::CsLocks(View<CriticalSection*> css) noexcept
	{
		_assert(css.size() == count);
		for (CriticalSection * cs : css)
		{
			m_cs.insert(cs);
		}
		for (CriticalSection * cs : m_cs)
		{
			cs->enter();
		}
	}
	template <size_t count>
	CsLocks<count>::~CsLocks() noexcept
	{
		for (CriticalSection * cs : m_cs.reverse())
		{
			cs->leave();
		}
	}

}
