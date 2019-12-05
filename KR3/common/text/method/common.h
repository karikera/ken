#pragma once

namespace kr
{
	namespace ary
	{
		template <class Parent> class CommonMethod
			:public Parent
		{
			CLASS_HEADER(CommonMethod, Parent);
		public:
			INHERIT_ARRAY();

			using Parent::Parent;

			void equalOperator(const CommonMethod& from)
			{
				this->~CommonMethod();
				new (this) CommonMethod(from);
			}
		};
	}
}
