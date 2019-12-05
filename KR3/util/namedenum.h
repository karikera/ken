#pragma once

#include <KR3/main.h>
#include <KR3/meta/text.h>

namespace kr
{
	namespace meta
	{
		template <typename ... TYPES>
		struct Caster;

		template <typename LAST>
		struct Caster<LAST>: LAST
		{
			inline Caster(LAST last) noexcept
				:LAST(last)
			{
			}
		};

		template <typename FIRST, typename SECOND, typename ... TYPES>
		struct Caster<FIRST, SECOND, TYPES ...>: Caster<SECOND, TYPES ...>
		{
			using Super = Caster<SECOND, TYPES ...>;
			inline Caster(FIRST first) noexcept
				:Super((SECOND)first)
			{
			}
		};
	}
	template <typename ENUM>
	class EnumClass:public Bufferable<EnumClass<ENUM>, BufferInfo<char, method::Memory, true, true>>
	{
	public:
		EnumClass() = default;
		inline EnumClass(ENUM value) noexcept;
		inline operator ENUM() const noexcept;
		inline Text getText() const noexcept;
		inline const char * $begin() const noexcept;
		inline const char * $end() const noexcept;
		inline size_t $size() const noexcept;

	private:
		template <size_t sz>
		struct Names
		{
			Text names[sz];

			inline Names(Text names) noexcept
			{
				size_t index = 0;
				for (Text item : names.splitIterable(','))
				{
					this->names[index++] = item.trim();
				}
			}
		};

		ENUM m_value;
	};

	template <typename ENUM>
	inline EnumClass<ENUM>::EnumClass(ENUM value) noexcept
		:m_value(value)
	{
	}
	template <typename ENUM>
	inline EnumClass<ENUM>::operator ENUM() const noexcept
	{
		return m_value;
	}
	template<typename ENUM>
	inline const char * EnumClass<ENUM>::$begin() const noexcept
	{
		return getText().begin();
	}
	template<typename ENUM>
	inline const char * EnumClass<ENUM>::$end() const noexcept
	{
		return getText().end();
	}
	template<typename ENUM>
	inline size_t EnumClass<ENUM>::$size() const noexcept
	{
		return getText().size();
	}
}
#define PR__NAMED_ENUM(v, i)	v(i)

#define named_enum_impl(name, ...) \
using name##Class = kr::EnumClass<name>; \
inline kr::Text kr::EnumClass<name>::getText() const noexcept { \
	static constexpr meta::strcount_t strcount(#__VA_ARGS__, ',');\
	static const Names<strcount.count> names(Text(strcount.str, strcount.size)); \
	return names.names[(size_t)m_value];\
}\
template <> \
struct kr::Bufferize<name, char> { using type = name##Class; };\
template <> \
struct kr::Bufferize<name, char16> { using type = meta::Caster<name##Class,AnsiToUtf16>; };

#define named_enum(name, ...) \
enum name{__VA_ARGS__} ;  \
named_enum_impl(name, __VA_ARGS__);

#define named_enum_class(name, ...) \
enum class name{__VA_ARGS__} ;  \
named_enum_impl(name, __VA_ARGS__);
