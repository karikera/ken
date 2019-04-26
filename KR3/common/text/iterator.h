#pragma once

namespace kr
{
	// 이터레이터의 끝
	struct IteratorEnd
	{
		template <typename T>
		friend bool operator != (const T & other, const IteratorEnd &) noexcept
		{
			return !other.isEnd();
		}
		template <typename T>
		friend bool operator == (const T & other, const IteratorEnd &) noexcept
		{
			return other.isEnd();
		}
	};

	template <typename This, typename Iterator, typename ... CTOR_PARAMETERS>
	class Iterable
	{
	private:
		const This * const m_ptr;
		const meta::types<CTOR_PARAMETERS...> m_params;

	public:
		Iterable(const This * ptr, const CTOR_PARAMETERS & ... params) noexcept
			: m_ptr(ptr), m_params(params...)
		{
		}
		Iterator begin() const noexcept
		{
			return Iterator(m_ptr, m_params);
		}
		IteratorEnd end() const noexcept
		{
			return IteratorEnd();
		}
	};

}
