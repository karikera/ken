#pragma once

#include "../main.h"

namespace kr
{
	template <typename T>
	class alignas(1) Unaligned
	{
	private:
		byte m_data[sizeof(T)];

	public:
		Unaligned() = default;
		Unaligned(const Unaligned& copy) = default;
		Unaligned(const T & v) noexcept
		{
			*this = (Unaligned&)v;
		}
		Unaligned& operator =(const T & v) noexcept
		{
			*this = (Unaligned&)v;
			return *this;
		}

		operator T() noexcept
		{
			T value;
			(Unaligned&)value = *this;
			return value;
		}
	};
}
