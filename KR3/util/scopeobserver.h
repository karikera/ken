#pragma once

#include <functional>

namespace kr
{
	namespace _pri_
	{
		template <typename CLOSER>
		class Finally
		{
		public:
			Finally(CLOSER closer) noexcept
				: m_closer(closer)
			{
			}
			~Finally() noexcept
			{
				m_closer();
			}

		private:
			CLOSER m_closer;
		};

		template <typename CLOSER> Finally<CLOSER> scope_helper(CLOSER closer)
		{
			return Finally<CLOSER>(closer);
		}
		template <typename OPENER, typename CLOSER> Finally<CLOSER> scope_helper(const OPENER & opener, CLOSER closer)
		{
			opener();
			return Finally<CLOSER>(closer);
		}
	}
}

#define kr_scope(opener, closer) auto UNIQUE(tmp) = ::kr::_pri_::scope_helper(opener, closer)
#define kr_finally(closer) auto UNIQUE(tmp) = ::kr::_pri_::scope_helper(closer)