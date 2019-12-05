#pragma once

namespace kr
{
	template <size_t size> class autovar final
	{
	protected:
		byte m_ptr[size];

	public:
		autovar() noexcept;
		template <typename T> autovar(const T &dat) noexcept;
		template <typename T> operator T&() noexcept;
		template <typename T> operator const T&() const noexcept;
		template <typename T> T& operator =(const T &dat) noexcept;
		template <typename T> bool operator ==(const T &dat) const noexcept;
		template <typename T> bool operator !=(const T &dat) const noexcept;
		template <typename T>
		friend bool operator ==(const T &dat, const autovar& th) noexcept
		{
			return dat != (T&)th.m_ptr;
		}
		template <typename T>
		friend bool operator !=(const T &dat, const autovar& th) noexcept
		{
			return dat != (T&)th.m_ptr;
		}

	};

	template<size_t size>
	inline autovar<size>::autovar() noexcept
	{
	}
	template<size_t size>
	template <typename T>
	inline autovar<size>::autovar(const T & dat) noexcept
	{
		*this = dat;
	}
	template <size_t size>
	template <typename T> 
	inline autovar<size>::operator T&() noexcept
	{
		static_assert(sizeof(T) <= size, "Not enough size");
		return (T&)m_ptr;
	};
	template <size_t size>
	template <typename T> 
	inline autovar<size>::operator const T&() const noexcept
	{
		static_assert(sizeof(T) <= size, "Not enough size");
		return (T&)m_ptr;
	}
	template <size_t size>
	template <typename T> 
	inline T& autovar<size>::operator =(const T &dat) noexcept
	{
		static_assert(sizeof(T) <= size, "Not enough size");
		return (T&)m_ptr = dat;
	}
	template <size_t size>
	template <typename T> 
	inline bool autovar<size>::operator ==(const T &dat) const noexcept
	{
		static_assert(sizeof(T) <= size, "Not enough size");
		return dat == (T&)m_ptr;
	}
	template <size_t size>
	template <typename T> 
	inline bool autovar<size>::operator !=(const T &dat) const noexcept
	{
		static_assert(sizeof(T) <= size, "Not enough size");
		return dat != (T&)m_ptr;
	}
}