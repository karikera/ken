#pragma once

#include "../main.h"

namespace kr
{
	template <typename ... ARGS>
	class Lister : private meta::types<add_pointer_t<ARGS> ...>
	{
		using Super = meta::types<add_pointer_t<ARGS> ...>;
	public:
		Lister(ARGS & ... args) noexcept
			:Super(&args ...)
		{
		}

		template <typename ITERABLE>
		Lister& operator =(ITERABLE & iterable) throws(EofException)
		{
			auto iter = iterable.begin();
			auto end = iterable.end();

			value_loop([&](auto * arg) {
				if (iter == end) throw EofException();
				*arg = *iter++;
			});
			return *this;
		}
	};

	template <typename ... ARGS>
	Lister<ARGS ...> list(ARGS & ... dest) noexcept
	{
		return Lister<ARGS ...>(dest ...);
	}
}
