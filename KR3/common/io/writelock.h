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
			S* const m_stream;
			T m_buffer[sz + (nullterm ? 1 : 0)];

		public:
			WriteLockImpl(S* stream) noexcept
				:m_stream(stream)
			{
			}
			~WriteLockImpl()
			{
				m_stream->write(m_buffer, sz);
			}
			T* begin() noexcept
			{
				return m_buffer;
			}
		};;
		template <typename S, typename T, bool nullterm>
		class WriteLockImpl<S, false, T, (size_t)-1, nullterm>
		{
		private:
			S* const m_stream;
			TmpArray<T> m_buffer;

		public:
			WriteLockImpl(S* stream, size_t sz) noexcept
				: m_stream(stream), m_buffer(sz,sz +(nullterm ? 1 : 0))
			{
			}
			~WriteLockImpl()
			{
				m_stream->write(m_buffer.begin(), m_buffer.size());
			}
			T* begin() noexcept
			{
				return m_buffer.begin();
			}
		};;
		template <typename S, typename T, size_t sz, bool nullterm> 
		class WriteLockImpl<S, true, T, sz, nullterm>
		{
		private:
			T * m_ptr;

		public:

			WriteLockImpl(S * stream) noexcept
			{
				stream->padding(sz + (nullterm ? 1 : 0));
				m_ptr = stream->prepare(sz);
			}
			~WriteLockImpl()
			{
			}
			T* begin() noexcept
			{
				return m_ptr;
			}
		};;
		template <typename S, typename T, bool nullterm> 
		class WriteLockImpl<S, true, T, (size_t)-1, nullterm>
		{
		private:
			T * m_ptr;
			size_t m_size;

		public:
			WriteLockImpl(S* stream, size_t sz) noexcept
			{
				m_size = sz;
				stream->padding(m_size + (nullterm ? 1 : 0));
				m_ptr = stream->prepare(m_size);
			}
			~WriteLockImpl()
			{
			}
			T* begin() noexcept
			{
				return m_ptr;
			}
		};;
	}

	template <class DATA, size_t size, bool nullterm> class WriteLock
		:public _pri_::WriteLockImpl<DATA, DATA::accessable, typename DATA::InternalComponent, size, nullterm>
	{
		static_assert(IsOStream<DATA>::value, "DATA is not OutStream");
		using Super = _pri_::WriteLockImpl<DATA, DATA::accessable, typename DATA::InternalComponent, size, nullterm>;
	public:
		using Super::Super;
		using Super::begin;

		typename DATA::Component& operator *() noexcept
		{
			return *begin();
		}
	};;

}