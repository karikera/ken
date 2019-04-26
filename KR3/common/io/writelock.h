#pragma once

#include <KR3/main.h>

namespace kr
{

	namespace _pri_
	{
		template <typename S, bool _accessable, typename T, size_t sz, bool nullterm>
		class WriteLockImpl;

		template <typename S, typename T, size_t sz, bool nullterm> 
		class WriteLockImpl<S, false, T, sz, nullterm>
		{
		private:
			T m_buffer[sz + (nullterm ? 1 : 0)];

		public:
			WriteLockImpl() noexcept
			{
			}
			T* begin() noexcept
			{
				return m_buffer;
			}
			size_t lock(S * stream) throws(NotEnoughSpaceException)
			{
			}
			void unlock(S* stream) throws(NotEnoughSpaceException)
			{
				stream->write(m_buffer, sz);
			}
		};;
		template <typename S, typename T, bool nullterm>
		class WriteLockImpl<S, false, T, (size_t)-1, nullterm>
		{
		private:
			TmpArray<T> m_buffer;

		public:
			WriteLockImpl(size_t sz) noexcept
				: m_buffer(sz,sz +(nullterm ? 1 : 0))
			{
			}
			~WriteLockImpl() noexcept
			{
			}
			T* begin() noexcept
			{
				return m_buffer.begin();
			}
			size_t lock(S * stream) throws(NotEnoughSpaceException)
			{
				return m_buffer.size();
			}
			void unlock(S * stream) throws(NotEnoughSpaceException)
			{
				stream->write(m_buffer.begin(), m_buffer.size());
			}
		};;
		template <typename S, typename T, size_t sz, bool nullterm> 
		class WriteLockImpl<S, true, T, sz, nullterm>
		{
		private:
			T * m_ptr;

		public:

			WriteLockImpl() noexcept
			{
			}
			T* begin() noexcept
			{
				return m_ptr;
			}
			size_t lock(S * stream) throws(NotEnoughSpaceException)
			{
				stream->padding(sz+(nullterm ? 1 : 0));
				m_ptr = stream->prepare(sz);
				return sz;
			}
			void unlock(S * stream) throws(NotEnoughSpaceException)
			{
			}
		};;
		template <typename S, typename T, bool nullterm> 
		class WriteLockImpl<S, true, T, (size_t)-1, nullterm>
		{
		private:
			T * m_ptr;
			size_t m_size;

		public:
			WriteLockImpl(size_t sz) noexcept
			{
				m_size = sz;
			}
			T* begin() noexcept
			{
				return m_ptr;
			}
			size_t lock(S * stream) throws(NotEnoughSpaceException)
			{
				stream->padding(m_size + (nullterm ? 1 : 0));
				m_ptr = stream->prepare(m_size);
				return m_size;
			}
			void unlock(S * stream) throws(NotEnoughSpaceException)
			{
			}
		};;
	}

	template <class DATA, size_t size, bool nullterm> class WriteLock
		:public _pri_::WriteLockImpl<DATA, DATA::accessable, typename DATA::Component, size, nullterm>
	{
		static_assert(IsOStream<DATA>::value, "DATA is not OutStream");
		using Super = _pri_::WriteLockImpl<DATA, DATA::accessable, typename DATA::Component, size, nullterm>;
	public:
		using Super::Super;
		using Super::begin;

		typename DATA::Component& operator *() noexcept
		{
			return *begin();
		}
	};;

}