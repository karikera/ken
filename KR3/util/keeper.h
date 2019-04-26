#pragma once

#include "../main.h"

#include <new>

namespace kr
{
	template <size_t SIZE, size_t COUNT> class BufferKeeper
	{
	private:
		size_t m_reserve;
		void* m_reserved[COUNT];

	public:
		static constexpr size_t bufferSize = SIZE;
		static constexpr size_t bufferCount = COUNT;

		BufferKeeper() noexcept
		{
			m_reserve = 0;
		}
		~BufferKeeper() noexcept
		{
			clear();
		}
		void* rawAlloc() noexcept
		{
			void * data;
			if (m_reserve != 0)
			{
				m_reserve--;
				data = m_reserved[m_reserve];
#ifndef NDEBUG
				memset(data, 0xcc, SIZE);
#endif
			}
			else
			{
				data = _new char[SIZE];
			}
			return data;
		}
		void rawFree(void * p) noexcept
		{
			_assert(p != nullptr);
			if (m_reserve != COUNT)
			{
#ifndef NDEBUG
				memset(p, 0xdd, SIZE);
#endif
				m_reserved[m_reserve] = p;
				m_reserve++;
				return;
			}

			delete[](char*)p;
		}
		void clear() noexcept
		{
			void ** p = m_reserved;
			void ** end = p + m_reserve;
			for (; p != end; p++)
			{
				delete[](char*)*p;
			}
			m_reserve = 0;
		}
	};

	template <size_t SIZE, size_t COUNT> class AtomicBufferKeeper
	{
	private:
		std::atomic<size_t> m_reserve;
		std::atomic<void*> m_reserved[COUNT];
		std::atomic<int> m_tester[COUNT];

	public:
		static constexpr size_t bufferSize = SIZE;
		static constexpr size_t bufferCount = COUNT;

		AtomicBufferKeeper() noexcept
			:m_reserve(0)
		{
			for (std::atomic<void *> & v : m_reserved)
			{
				v = nullptr;
			}
		}
		~AtomicBufferKeeper() noexcept
		{
			clear();
		}
		void* rawAlloc() noexcept
		{
			void * data;

			for (;;)
			{
				size_t cmp = m_reserve;
				if (cmp == 0)
				{
					return malloc(SIZE);
				}
				size_t next = cmp - 1;
				if (m_reserve.compare_exchange_weak(cmp, next))
				{
					m_tester[next] --;
					std::atomic<void*>& atom = m_reserved[next];
					for (;;)
					{
						data = atom.exchange(nullptr);
						if (data == nullptr) continue;
						break;
					}
#ifndef NDEBUG
					memset(data, 0xcc, SIZE);
#endif
					return data;
				}
			}
			return data;
		}
		void rawFree(void * p) noexcept
		{
			_assert(p != nullptr);
			for (;;)
			{
				size_t cmp = m_reserve;
				if (cmp == COUNT)
				{
					free(p);
					return;
				}
				if (m_reserve.compare_exchange_weak(cmp, cmp + 1))
				{
					m_tester[cmp] ++;
#ifndef NDEBUG
					memset(p, 0xdd, SIZE);
#endif
					std::atomic<void*>& atom = m_reserved[cmp];
					void * p_cmp;
					do
					{
						p_cmp = nullptr;
					} while (!atom.compare_exchange_weak(p_cmp, p));
					return;
				}
			}

		}
		void clear() noexcept
		{
			size_t cmp = m_reserve;
			while (cmp != 0)
			{
				size_t next = cmp - 1;
				if (m_reserve.compare_exchange_weak(cmp, next))
				{
					std::atomic<void*>& atom = m_reserved[next];
					for (;;)
					{
						void * data = atom.exchange(nullptr);
						if (data == nullptr) continue;
						free(data);
						break;
					}
					cmp = next;
				}
				else
				{
					cmp = m_reserve;
				}
			}
		}
	};

	template <typename T, typename KeeperBase> class ObjKeeperCommon:public KeeperBase
	{
	public:
		using KeeperBase::rawAlloc;
		using KeeperBase::rawFree;

		template <typename ... ARGS> T * alloc(ARGS && ... args) noexcept
		{
			return ::new(rawAlloc()) T(move(args) ...);
		}
		void free(T * ptr) noexcept
		{
			ptr->~T();
			rawFree(ptr);
		}
	};


	template <typename KeeperBase> class SharingKeeperCommon:public KeeperBase
	{
	public:
		using KeeperBase::rawAlloc;
		using KeeperBase::rawFree;
		using KeeperBase::bufferSize;

		template <typename T, typename ... ARGS>
		T * alloc(ARGS && ... args) noexcept
		{
			static_assert(sizeof(T) <= bufferSize, "Too big size class");

			return ::new(rawAlloc()) T(forward<ARGS>(args) ...);
		}
		template <typename T>
		void free(T * ptr) noexcept
		{
			ptr->~T();
			rawFree(ptr);
		}
	};

	template <typename T, size_t COUNT>
	using Keeper = ObjKeeperCommon<T, BufferKeeper<sizeof(T), COUNT>>;

	template <typename T, size_t COUNT>
	using AtomicKeeper = ObjKeeperCommon<T, AtomicBufferKeeper<sizeof(T), COUNT>>;

	template <size_t SIZE, size_t COUNT>
	using SharingKeeper = SharingKeeperCommon<BufferKeeper<SIZE, COUNT>>;

	template <size_t SIZE, size_t COUNT>
	using AtomicSharingKeeper = SharingKeeperCommon<AtomicBufferKeeper<SIZE, COUNT>>;
}
