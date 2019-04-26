#pragma once

#include "../main.h"

namespace kr
{
	template <typename T, int id = 0> struct NumberClass;
}
template <typename T, int id> struct std::is_class<kr::NumberClass<T, id>>: std::false_type
{
};
template <typename T, int id> struct std::is_arithmetic<kr::NumberClass<T, id>> : std::true_type
{
};
template <typename T, int id> struct std::is_unsigned<kr::NumberClass<T, id>>: std::is_unsigned<T>
{
};
template <typename T, int id> struct std::make_signed<kr::NumberClass<T, id>> : std::make_signed<T>
{
};

namespace kr
{

	template <typename T, int id> struct NumberClass
	{
		static_assert(std::is_arithmetic<T>::value, "T is not number");
	public:
		NumberClass() = default;
		NumberClass(T value) noexcept;
		operator T() const noexcept;
		T* operator &() noexcept;
		const T* operator &() const noexcept;
		T& operator =(T value) noexcept;
		T& operator +=(T value) noexcept;
		T& operator -=(T value) noexcept;
		T& operator *=(T value) noexcept;
		T& operator /=(T value) noexcept;
		T& operator %=(T value) noexcept;
		T& operator &=(T value) noexcept;
		T& operator |=(T value) noexcept;
		T& operator ^=(T value) noexcept;

	private:
		T m_value;
	};

}

template <typename T, int id> kr::NumberClass<T,id>::NumberClass(T value) noexcept:m_value(value)
{
}
template <typename T, int id> kr::NumberClass<T,id>::operator T() const noexcept
{
	return m_value;
}
template <typename T, int id> T* kr::NumberClass<T,id>::operator &() noexcept
{
	return &m_value;
}
template <typename T, int id> const T* kr::NumberClass<T,id>::operator &() const noexcept
{
	return &m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator =(T value) noexcept
{
	m_value = value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator +=(T value) noexcept
{
	m_value += value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator -=(T value) noexcept
{
	m_value -= value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator *=(T value) noexcept
{
	m_value *= value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator /=(T value) noexcept
{
	m_value /= value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator %=(T value) noexcept
{
	m_value %= value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator &=(T value) noexcept
{
	m_value &= value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator |=(T value) noexcept
{
	m_value |= value;
	return m_value;
}
template <typename T, int id> T& kr::NumberClass<T,id>::operator ^=(T value) noexcept
{
	m_value ^= value;
	return m_value;
}
