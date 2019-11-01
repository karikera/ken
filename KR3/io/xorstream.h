#pragma once

#include "../main.h"

namespace kr
{
	namespace io
	{
		template <typename OS>
		class XorOStream:public OutStream<XorOStream<OS>, StreamInfo<typename OS::Component>>
		{
			static_assert(IsOStream<OS>::value, "OS is not OutStream");

		private:
			OS * m_os;
			dword m_key;
			uintp_t m_offset;

		public:
			XorOStream(OS * os, dword key) noexcept
				:m_os(os), m_key(key)
			{
				m_offset = 0;
			}

			void $write(typename const OS::Component * data, size_t sz)
			{
				WriteLock<OS> lock(sz);
				size_t size = lock.lock(m_os);
				mem::xor_copy();
				lock.begin();
				lock.unlock(m_os);
			}
		};
		template <typename IS>
		class XorIStream :public InStream<XorIStream<IS>, StreamInfo<typename IS::Component>>
		{
			static_assert(IsIStream<IS>::value, "IS is not InStream");

		private:
			IS * m_is;
			dword m_key;
			uintp_t m_offset;

		public:
			XorIStream(IS * is, dword key) noexcept
				: m_is(is), m_key(key)
			{
				m_offset = 0;
			}

			size_t $read(typename IS::Component * data, size_t sz)
			{
				return m_is->read(data, sz);
			}
		};
	}
}