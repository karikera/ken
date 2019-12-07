#pragma once

#include "../hasmethod.h"
#include "membuffer.h"
#include "textbuffer.h"

namespace kr
{
	namespace method
	{
		namespace _pri_
		{
			template <class Derived, bool _string, bool _readonly, typename Info>
			struct MemorySwitch;
			template <class Derived, typename Info>
			struct MemorySwitch<Derived, false, true, Info>
			{
				using type = buffer::Memory<Derived, Info>;
			};
			template <class Derived, typename Info>
			struct MemorySwitch<Derived, false, false, Info>
			{
				using type = buffer::WMemory<Derived, Info>;
			};
			template <class Derived, typename Info>
			struct MemorySwitch<Derived, true, true, Info>
			{
				using type = buffer::Text<Derived, Info>;
			};
			template <class Derived, typename Info>
			struct MemorySwitch<Derived, true, false, Info>
			{
				using type = buffer::WText<Derived, Info>;
			};
		}
		template <typename Derived, typename Info>
		class Memory : public _pri_::MemorySwitch<Derived, Info::is_string, Info::readonly, Info>::type
		{
			static_assert(!is_same<typename Info::Component, AutoComponent>::value, "Memory buffer cannot be AutoComponent");
			using Super = typename _pri_::MemorySwitch<Derived, Info::is_string, Info::readonly, Info>::type;
		public:
			using Super::Super;
		};
		template <typename Derived, typename Info>
		class CopyTo: public WriteToByCopyTo<Derived, typename Info::Component, Info>
		{
			using Super = WriteToByCopyTo<Derived, typename Info::Component, Info>;
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};;
		template <typename Derived, typename Info>
		class WriteTo: public CopyToByWriteTo<Derived, typename Info::Component, Info>
		{
			using Super = CopyToByWriteTo<Derived, typename Info::Component, Info>;
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};
		template <typename Derived, typename Info>
		class CopyWriteTo:public HasCopyWriteTo<Derived, typename Info::Component, Info>
		{
			using Super = HasCopyWriteTo<Derived, typename Info::Component, Info>;
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};
	}
}