#pragma once

#include <allocators>
#include <malloc.h>

namespace kr
{
	template <typename T>
	struct RawAllocator
	{
		RawAllocator() = default;
		RawAllocator(const RawAllocator&) = default;

		void deallocate(T * const _Ptr, const size_t _Count) noexcept
		{
			::free(_Ptr);
		}

		T * allocate(const size_t _Count) noexcept
		{
			return (T*)malloc(_Count * sizeof(T));
		}
		T * allocate(const size_t _Count, const void *) noexcept
		{
			return (allocate(_Count));
		}

		template<class... _Types>
		void construct(T * const _Ptr, _Types&&... _Args) noexcept
		{
			::new (_Ptr) T(std::forward<_Types>(_Args)...);
		}

		void destroy(T * const _Ptr) noexcept
		{
			_Ptr->~T();
		}

		size_t max_size() const noexcept
		{
			return ((size_t)-1) / sizeof(T);
		}
	};
}
