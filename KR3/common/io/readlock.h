#pragma once

#include <KR3/main.h>

namespace kr
{

	namespace _pri_
	{
		template <bool _accessable, typename T, size_t sz> 
		class ReadLockImpl;

		template <typename T, size_t sz> 
		class ReadLockImpl<false, T, sz>
		{
		private:
			T m_buffer[sz];

		public:
			ReadLockImpl() noexcept
			{
			}
			const T* begin() noexcept
			{
				return m_buffer;
			}
			template <typename IS>
			size_t lock(IS * stream) throws(EofException)
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
				return stream->read(m_buffer, sz);
			}
			template <typename IS>
			void unlock(IS*) noexcept
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
			}
		};;
		template <typename T>
		class ReadLockImpl<false, T, (size_t)-1>
		{
		private:
			Array<T> m_buffer;

		public:

			ReadLockImpl(size_t sz) noexcept
				: m_buffer(sz)
			{
			}
			~ReadLockImpl() noexcept
			{
			}
			const T* begin() noexcept
			{
				return (T*)m_buffer;
			}
			template <typename IS> 
			size_t lock(IS * stream) throws(EofException)
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
				return stream->read((T*)m_buffer, m_buffer.size());
			}
			template <typename IS> 
			void unlock(IS * stream) noexcept
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
			}
		};;
		template <typename T, size_t sz> 
		class ReadLockImpl<true, T, sz>
		{
		private:
			T * m_ptr;

		public:

			ReadLockImpl() noexcept
			{
			}
			const T* begin() noexcept
			{
				return m_ptr;
			}
			template <typename IS>
			size_t lock(IS * stream) throws(EofException)
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
				size_t size = sz;
				m_ptr = stream->read(&size);
				return size;
			}
			template <typename IS>
			void unlock(IS * stream) noexcept
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
			}
		};;
		template <typename T> 
		class ReadLockImpl<true, T, (size_t)-1>
		{
		private:
			T * m_ptr;
			size_t m_size;

		public:
			ReadLockImpl(size_t sz) noexcept
			{
				m_size = sz;
			}
			const T* begin() noexcept
			{
				return m_ptr;
			}
			template <typename IS>
			size_t lock(IS * stream) throws(EofException)
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
				size_t size = m_size;
				m_ptr = stream->read(&size);
				return size;
			}
			template <typename IS> 
			void unlock(IS * stream) noexcept
			{
				static_assert(IsIStream<IS>::value, "IS is not istream");
			}
		};;
	}

	template <class DATA, size_t size> class ReadLock
		:public _pri_::ReadLockImpl<DATA::lockType, typename DATA::Component, size>
	{
		static_assert(IsIStream<DATA>::value, "DATA is not InStream");
		using Super = _pri_::ReadLockImpl<DATA::lockType, typename DATA::Component, size>;
	public:
		using Component = typename DATA::Component;

		using Super::begin;
		using Super::Super;
		const Component& operator *() noexcept
		{
			return *begin();
		}
	};;


}