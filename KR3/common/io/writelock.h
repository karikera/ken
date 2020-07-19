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
			T* lock(S* stream) noexcept
			{
				return m_buffer;
			}
			void unlock(S* stream, size_t commit) throws(...)
			{
				_assert(commit <= sz);
				stream->write(m_buffer, commit);
			}
			void unlock(S* stream) throws(...)
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
			{
				m_buffer.resize(sz, sz + (nullterm ? 1 : 0));
			}
			T* lock(S* stream) noexcept
			{
				return m_buffer.begin();
			}
			void unlock(S* stream, size_t commit) throws(...)
			{
				_assert(commit <= m_buffer.size());
				stream->write(m_buffer.begin(), commit);
			}
			void unlock(S* stream) throws(...)
			{
				stream->write(m_buffer.begin(), m_buffer.size());
			}
		};;
		template <typename S, typename T, size_t sz, bool nullterm> 
		class WriteLockImpl<S, true, T, sz, nullterm>
		{
		public:
			T* lock(S* stream) throws(...)
			{
				return stream->padding(sz + (nullterm ? 1 : 0));
			}
			void unlock(S* stream, size_t commit) noexcept
			{
				_assert(commit <= sz);
				stream->commit(commit);
			}
			void unlock(S* stream) noexcept
			{
				stream->commit(sz);
			}
		};;
		template <typename S, typename T, bool nullterm> 
		class WriteLockImpl<S, true, T, (size_t)-1, nullterm>
		{
		private:
			size_t m_size;

		public:
			WriteLockImpl(size_t sz) noexcept
				: m_size(sz)
			{
			}

			T* lock(S* stream) noexcept
			{
				return (T*)stream->padding(m_size + (nullterm ? 1 : 0));
			}
			void unlock(S* stream, size_t commit) noexcept
			{
				_assert(commit <= m_size);
				stream->commit(commit);
			}
			void unlock(S* stream) noexcept
			{
				stream->commit(m_size);
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
		using Super::lock;
		using Super::unlock;
	};;

}