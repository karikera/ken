#pragma once

namespace kr
{
	namespace _pri_
	{
		template <typename T> class FinallyClass
		{
		public:
			inline FinallyClass(T lambda) noexcept;
			inline ~FinallyClass();

		private:
			T m_lambda;
		};
		template <typename T> 
		inline FinallyClass<T>::FinallyClass(T lambda) noexcept
			:m_lambda(lambda)
		{
		}
		template <typename T> 
		inline FinallyClass<T>::~FinallyClass()
		{
			m_lambda();
		}

		class StaticCode
		{
		public:
			template <typename LAMBDA>
			inline StaticCode(LAMBDA lambda)
			{
				lambda();
			}
		};

		struct MakeFinallyHelper
		{
			MakeFinallyHelper() = delete;
			~MakeFinallyHelper() = delete;
			MakeFinallyHelper(const MakeFinallyHelper&) = delete;
			MakeFinallyHelper& operator =(const MakeFinallyHelper&) = delete;

			template <typename LAMBDA>
			FinallyClass<LAMBDA> operator +(LAMBDA&& lambda) const noexcept
			{
				return lambda;
			}
		};

		static const MakeFinallyHelper & makeFinallyHelper = nullref;
	}
}

#define staticCode	\
	static kr::_pri_::StaticCode UNIQUE(STATIC_LAMBDA) = []
#define finally		\
	auto UNIQUE(FINALLY) = kr::_pri_::makeFinallyHelper + [&]

