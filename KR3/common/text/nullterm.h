#pragma once

#include "buffer.h"

namespace kr
{
	
	template <typename Buffer>
	class ToSZ : public Bufferable<ToSZ<Buffer>, BufferInfo<typename Buffer::Component, method::CopyTo, true, true>>
	{
		static_assert(IsBuffer<Buffer>::value, "Buffer is not buffer");
		using Super = Bufferable<ToSZ<Buffer>, BufferInfo<typename Buffer::Component, method::CopyTo, true, true>>;
	public:
		using typename Super::Component;

		ToSZ(const Buffer& buffer) noexcept
			: m_buffer(buffer)
		{
		}
		size_t $copyTo(typename Buffer::Component * dest) const noexcept
		{
			size_t sz = m_buffer.copyTo(dest);
			if (!Buffer::szable)
				dest[sz] = (Component)0;
			return sz;
		}
		size_t $size() const noexcept
		{
			return m_buffer.size();
		}

	private:
		const Buffer& m_buffer;
	};;

	template <class Buffer>
	ToSZ<Buffer> toSz(const Buffer &buffer) noexcept
	{
		return buffer;
	}
	

}