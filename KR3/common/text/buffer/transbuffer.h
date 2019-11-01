#pragma once

#include "commoncls.h"

namespace kr
{
	namespace buffer
	{
		template <typename Component, class Derived, typename Info>
		class TransBuffer;
		template <typename Component, class Derived, bool szable, typename Parent>
		class TransBuffer<Component, Derived, BufferInfo<Component, true, false, szable, true, Parent> >
			: public _pri_::CopyToOnlyImpl<Derived, Component, BufferInfo<Component, true, false, szable, true, Parent> >
		{
			using Super = _pri_::CopyToOnlyImpl<Derived, Component, BufferInfo<Component, true, false, szable, true, Parent> >;
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};
		template <typename Component, class Derived, bool szable, typename Parent>
		class TransBuffer<Component, Derived, BufferInfo<Component, false, true, szable, true, Parent> >
			: public _pri_::WriteToOnlyImpl<Derived, Component, BufferInfo<Component, false, true, szable, true, Parent> >
		{
			using Super = _pri_::WriteToOnlyImpl<Derived, Component, BufferInfo<Component, false, true, szable, true, Parent> >;
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};
		template <typename Component, class Derived, bool szable, typename Parent>
		class TransBuffer<Component, Derived, BufferInfo<Component, true, true, szable, true, Parent> >
			: public _pri_::CopyToWriteToImpl<Derived, Component, BufferInfo<Component, true, true, szable, true, Parent> >
		{
			using Super = _pri_::CopyToWriteToImpl<Derived, Component, BufferInfo<Component, true, true, szable, true, Parent> >;
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};
	}
}