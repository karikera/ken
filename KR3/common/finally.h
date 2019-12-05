#pragma once

namespace kr
{
	namespace _pri_
	{
		template <typename LAMBDA> class FinallyClass
		{
		public:
			inline FinallyClass(LAMBDA&& lambda) noexcept;
			inline ~FinallyClass();

		private:
			LAMBDA m_lambda;
		};
		template <typename LAMBDA> 
		inline FinallyClass<LAMBDA>::FinallyClass(LAMBDA&& lambda) noexcept
			:m_lambda(move(lambda))
		{
		}
		template <typename LAMBDA> 
		inline FinallyClass<LAMBDA>::~FinallyClass()
		{
			m_lambda();
		}

		class StaticCode
		{
		public:
			template <typename LAMBDA>
			inline StaticCode(LAMBDA &&lambda)
			{
				lambda();
			}
		};

		struct MakeFinallyHelper final
		{
			template <typename LAMBDA>
			FinallyClass<decay_t<LAMBDA> > operator +(LAMBDA &&lambda) const noexcept
			{
				return forward<LAMBDA>(lambda);
			}
		};

		static constexpr const MakeFinallyHelper makeFinallyHelper = MakeFinallyHelper();
	}
}

#define staticCode	\
	static kr::_pri_::StaticCode UNIQUE(STATIC_LAMBDA) = []
#define finally		\
	auto UNIQUE(FINALLY) = kr::_pri_::makeFinallyHelper + [&]

