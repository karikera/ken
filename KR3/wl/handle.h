#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include "../main.h"

namespace kr
{
	namespace _pri_
	{
		void closeHandle(void * handle) noexcept;
	}
	template <typename T> class Handle:public T
	{
	public:
		Handle() = delete;

		inline void operator delete(void * v) noexcept
		{
			_pri_::closeHandle(v);
		}
	};
	template <> class Handle<void>
	{
	protected:
		const void* const __junk;

	public:
		Handle() = delete;
		inline void operator delete(void * v) noexcept
		{
			_pri_::closeHandle(v);
		}
	};;

	class ihv_t
	{
		inline bool operator ==(cptr v) noexcept
		{
			return v == (cptr) - 1;
		}
		inline bool operator !=(cptr v) noexcept
		{
			return v != (cptr) - 1;
		}
		inline friend bool operator ==(cptr v, ihv_t&) noexcept
		{
			return v == (cptr) - 1;
		}
		inline friend bool operator !=(cptr v, ihv_t&) noexcept
		{
			return v != (cptr) - 1;
		}
	};
	static ihv_t &ihv = nullref;
}
