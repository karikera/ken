#pragma once

#include <type_traits>

namespace kr
{
	
	template <typename PTR>
	class autoptr_t final
	{
	public:
		using UPTR = std::make_unsigned_t<PTR>;
		using SPTR = std::make_signed_t<PTR>;

		autoptr_t() = default;
		autoptr_t(nullptr_t) noexcept;
		autoptr_t& operator =(nullptr_t) noexcept;
		explicit autoptr_t(UPTR dat) noexcept;
		explicit autoptr_t(SPTR dat) noexcept;
		template <typename T> autoptr_t(T* dat) noexcept;
		template <typename T> operator T*() const noexcept;
		explicit operator PTR() const noexcept;
		template <typename T> autoptr_t& operator =(T* dat) noexcept;
		template <typename T> bool operator ==(T * dat) const noexcept;
		template <typename T> bool operator !=(T * dat) const noexcept;
		bool operator ==(nullptr_t) const noexcept;
		bool operator !=(nullptr_t) const noexcept;
		friend bool operator ==(nullptr_t, const autoptr_t& _this) noexcept;
		friend bool operator !=(nullptr_t, const autoptr_t& _this) noexcept;

	protected:
		PTR m_ptr;
	};

	template <typename PTR>
	autoptr_t<PTR>::autoptr_t(nullptr_t) noexcept
		:m_ptr(0)
	{
	}
	template <typename PTR>
	autoptr_t<PTR>::autoptr_t(UPTR dat) noexcept
		:m_ptr(dat)
	{
	}
	template <typename PTR>
	autoptr_t<PTR>::autoptr_t(SPTR dat) noexcept
		:m_ptr(dat)
	{
	}
	template <typename PTR>
	autoptr_t<PTR>& autoptr_t<PTR>::operator=(nullptr_t) noexcept
	{
		m_ptr = 0;
		return *this;
	}
	template <typename PTR>
	bool autoptr_t<PTR>::operator ==(nullptr_t) const noexcept
	{
		return m_ptr == 0;
	}
	template <typename PTR>
	bool autoptr_t<PTR>::operator !=(nullptr_t) const noexcept
	{
		return m_ptr != 0;
	}
	template <typename PTR>
	bool operator ==(nullptr_t, const autoptr_t<PTR>& _this) noexcept
	{
		return _this == 0;
	}
	template <typename PTR>
	bool operator !=(nullptr_t, const autoptr_t<PTR>& _this) noexcept
	{
		return _this != 0;
	}

	template <typename PTR>
	template <typename T>
	autoptr_t<PTR>::autoptr_t(T * dat) noexcept
	{
		*this = dat;
	}
	template <typename PTR>
	template <typename T>
	autoptr_t<PTR>::operator T*() const noexcept
	{
		return (T*)(uintptr_t)m_ptr;
	}
	template <typename PTR>
	autoptr_t<PTR>::operator PTR() const noexcept
	{
		return m_ptr;
	}
	template <typename PTR>
	template <typename T>
	autoptr_t<PTR>& autoptr_t<PTR>::operator =(T* dat) noexcept
	{
		m_ptr = (PTR)dat;
		return *this;
	}
	template <typename PTR>
	template <typename T>
	bool autoptr_t<PTR>::operator ==(T * dat) const noexcept
	{
		return m_ptr == dat;
	}
	template <typename PTR>
	template <typename T>
	bool autoptr_t<PTR>::operator !=(T * dat) const noexcept
	{
		return m_ptr != dat;
	}

	using autoptr = autoptr_t<uintptr_t>;
	using autoptr64 = autoptr_t<uint64_t>;
}

extern template class kr::autoptr_t<uintptr_t>;
#ifndef _WIN64
extern template class kr::autoptr_t<uint64_t>;
#endif