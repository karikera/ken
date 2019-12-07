#pragma once

#include <KR3/main.h>

namespace kr
{

	class StackGC
	{
	private:
		struct Head
		{
			Head* prev;
			void (*destruct)(void* data);
		};
		Head* m_last;

	public:
		StackGC() noexcept;
		~StackGC() noexcept;

		template <typename T, typename ... ARGS>
		T* allocate(ARGS&& ... args) noexcept
		{
			StackAllocator* alloc = StackAllocator::getInstance();
			Head* head = alloc->allocate(sizeof(T), alignof(T), sizeof(Head));
			head->prev = m_last;
			m_last = head;
			m_last->destruct = [](void* data) {
				((T*)data)->~T();
			};
			return new((T*)(head + 1)) T(forward<ARGS>(args) ...);
		}
	};

}
