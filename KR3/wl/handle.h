#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/main.h>
#include <KR3/util/path.h>

#include "windows.h"

typedef struct HINSTANCE__* HINSTANCE;

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

	namespace win
	{
		class Library :public Handle<HINSTANCE__>
		{
		public:
			static Library* load(pcstr16 str) noexcept;
			static Module* byName(pcstr16 str) noexcept;
			static Module* current() noexcept;
			void operator delete(void* library) noexcept;
			const autovar<sizeof(ptr)> get(pcstr str) noexcept;
			template <typename T> size_t getFileName(T* dest, size_t capacity) const noexcept;
			template <typename T> size_t getFileNameLength() const noexcept;

			KR_WRITABLE_METHOD(Library, name, true, (m_this->getFileName<C>(dest, Path::MAX_LEN)), (m_this->getFileNameLength<C>()));
		};
	}

	class ihv_t final
	{
	public:
		inline bool operator ==(cptr v) const noexcept
		{
			return v == (cptr) - 1;
		}
		inline bool operator !=(cptr v) const noexcept
		{
			return v != (cptr) - 1;
		}
		inline friend bool operator ==(cptr v, const ihv_t&) noexcept
		{
			return v == (cptr) - 1;
		}
		inline friend bool operator !=(cptr v, const ihv_t&) noexcept
		{
			return v != (cptr) - 1;
		}
	};
	static constexpr const ihv_t ihv = ihv_t();
}
