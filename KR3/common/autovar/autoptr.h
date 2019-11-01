#pragma once

namespace kr
{
	
	class autoptr final
	{
	public:
		autoptr() = default;
		autoptr(nullptr_t) noexcept;
		autoptr& operator =(nullptr_t) noexcept;
		explicit autoptr(intptr_t dat) noexcept;
		explicit autoptr(uintptr_t dat) noexcept;
		template <typename T> autoptr(T* dat) noexcept;
		template <typename T> operator T*() const noexcept;
		template <typename T> autoptr& operator =(T* dat) noexcept;
		template <typename T> bool operator ==(T * dat) const noexcept;
		template <typename T> bool operator !=(T * dat) const noexcept;
		bool operator ==(nullptr_t) const noexcept;
		bool operator !=(nullptr_t) const noexcept;
		friend bool operator ==(nullptr_t, const autoptr& _this) noexcept;
		friend bool operator !=(nullptr_t, const autoptr& _this) noexcept;

	protected:
		ptr m_ptr;
	};

	template <typename T>
	inline autoptr::autoptr(T * dat) noexcept
	{
		*this = dat;
	}
	template <typename T>
	inline autoptr::operator T*() const noexcept
	{
		return (T*)m_ptr;
	}
	template <typename T>
	inline autoptr& autoptr::operator =(T* dat) noexcept
	{
		m_ptr = (ptr)dat;
		return *this;
	}
	template <typename T>
	inline bool autoptr::operator ==(T * dat) const noexcept
	{
		return m_ptr == dat;
	}
	template <typename T>
	inline bool autoptr::operator !=(T * dat) const noexcept
	{
		return m_ptr != dat;
	}
}