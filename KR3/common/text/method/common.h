#pragma once

namespace kr
{
	namespace ary
	{
		namespace method
		{
			template <class Parent> class Common
				:public Parent
			{
				CLASS_HEADER(Common, Parent);
			public:
				INHERIT_ARRAY();

				using Parent::Parent;

				void equalOperator(const Common & from)
				{
					this->~Common();
					new (this) Common(from);
				}
			};
		}
	}
}
