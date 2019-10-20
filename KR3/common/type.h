#pragma once

#include "compiler.h"

#include <type_traits>
#include <locale>
#include <ostream>

using std::max_align_t;

typedef unsigned char byte;

namespace kr
{
	struct Undefined;

	// kr type
	using uint = unsigned int;
	using qword = uint64_t;
	using dword = uint32_t;
	using word = uint16_t;
	using ::byte;
	using filesize_t = uint64_t;

	using ptr = void *;
	using cptr = const void *;

	using pstr = ATTR_NULL_TERMINATED char*;
	using pcstr = ATTR_NULL_TERMINATED const char*;

	using char16 = char16_t;
	using pstr16 = ATTR_NULL_TERMINATED char16*;
	using pcstr16 = ATTR_NULL_TERMINATED const char16*;
	
	using char32 = char32_t;
	using pstr32 = ATTR_NULL_TERMINATED char32*;
	using pcstr32 = ATTR_NULL_TERMINATED const char32*;
		
	template <typename T, size_t if_void>
	struct sizeof_with_void { static constexpr size_t value = sizeof(T);  };
	template <size_t if_void>
	struct sizeof_with_void<void, if_void> { static constexpr size_t value = if_void; };
	
	#undef ptr_def
	#undef ptr_typedef

	constexpr int eof = -1;
		
	class nullref_t final
	{
	public:
		nullref_t() = delete;
		~nullref_t() = delete;
		nullref_t(const nullref_t&) = delete;
		nullref_t& operator =(const nullref_t&) = delete;

		template <typename T> bool operator ==(const T &t) const noexcept
		{
			return &t == nullptr;
		}
		template <typename T> bool operator !=(const T &t) const noexcept
		{
			return &t != nullptr;
		}
		template <typename T> operator T&() const noexcept
		{
#pragma warning(push)
#pragma warning(disable:6011)
			return *(T*)(1-1);
#pragma warning(pop)
		}
		template <typename T> friend bool operator ==(const T &t, const nullref_t&) noexcept
		{
			return &t == nullptr;
		}
		template <typename T> friend bool operator !=(const T &t, const nullref_t&) noexcept
		{
			return &t != nullptr;
		}
	};
	
	static const nullref_t &nullref = *(nullref_t*)(1-1);

	using std::endl;


	struct Range
	{
		size_t beg, end;
	};

	enum class Axis
	{
		X,Y,Z,W
	};
}

using std::nullptr_t;
using kr::char16;
using kr::char32;

namespace kr
{
	namespace type
	{
		using ::kr::uint;
		using ::kr::qword;
		using ::kr::dword;
		using ::kr::word;
		using ::kr::byte;
		using ::kr::ptr;	using ::kr::cptr;
		using ::kr::pstr;	using ::kr::pcstr;
		using ::kr::pstr16;	using ::kr::pcstr16;
		using ::kr::pstr32;	using ::kr::pcstr32;
		using ::kr::filesize_t;
	}
}