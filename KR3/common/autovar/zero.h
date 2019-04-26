#pragma once

#ifndef __KR3_INCLUDED
#error Need To Include KR3/main.h
#endif

namespace kr
{
	namespace _pri_
	{
		struct EMPTY_MEM_STRUCT
		{
			size_t buffer[16/sizeof(size_t)];
		};
		extern const EMPTY_MEM_STRUCT ZERO_MEMORY;
		template <typename T> struct ZeroVarValueRef
		{
			using type = T&;
		};
		template <> struct ZeroVarValueRef<void>
		{
			using type = void;
		};
	}
	class zerovar_t final
	{
	public:
		zerovar_t() noexcept;
		template <typename T> 
		ATTR_CHECK_RETURN typename _pri_::ZeroVarValueRef<T>::type value() const noexcept;
		template <typename T> 
		operator T&() const noexcept;
	};

	template <> void zerovar_t::value<void>() const noexcept;
	
	class zeroptr_t final
	{
	public:
		zeroptr_t() noexcept;
		template <typename T> operator T*() const noexcept;
	};

	static zerovar_t &zerovar = nullref;
	static zeroptr_t &zeroptr = nullref;
	template <typename T>
	inline typename _pri_::ZeroVarValueRef<T>::type zerovar_t::value() const noexcept
	{
		return (T&)_pri_::ZERO_MEMORY;
	}
	template <> 
	inline void zerovar_t::value<void>() const noexcept
	{
	}
	template <typename T> 
	inline zerovar_t::operator T&() const noexcept
	{
		static_assert(sizeof(T) <= sizeof(_pri_::ZERO_MEMORY), "T size bigger than ZERO_MEMORY");
		return (T&)_pri_::ZERO_MEMORY;
	}
	template <typename T> 
	inline zeroptr_t::operator T*() const noexcept
	{
		static_assert(sizeof(T) <= sizeof(_pri_::ZERO_MEMORY), "T size bigger than ZERO_MEMORY");
		return (T*)&_pri_::ZERO_MEMORY;
	}
}