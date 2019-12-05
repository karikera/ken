#pragma once

#include "container.h"
#include "method/common.h"
#include "method/buffer_i.h"
#include "method/buffer_io.h"
#include "method/stream.h"
#include "../../meta/ordering.h"

namespace kr
{
	namespace ary
	{
		template <class Data, typename C> 
		class WrapImpl :
			public meta::crtp_order<Data>::
			template attach<ary::CommonMethod, true>::
			template attach<ary::OStreamMethod, Data::extendable>::
			template attach<ary::IStreamMethod, Data::movable>::
			template attach<ary::IRStreamMethod, Data::movable>::
			template attach<ary::BufferIMethod, Data::readable>::
			template attach<ary::BufferIOMethod, Data::extendable && Data::readable>::
			type
		{
			CLASS_HEADER(WrapImpl,
				typename meta::crtp_order<Data>::
				template attach<ary::CommonMethod, true>::
				template attach<ary::OStreamMethod, Data::extendable>::
				template attach<ary::IStreamMethod, Data::movable>::
				template attach<ary::IRStreamMethod, Data::movable>::
				template attach<ary::BufferIMethod, Data::readable>::
				template attach<ary::BufferIOMethod, Data::extendable && Data::readable>::
				type);
		public:
			INHERIT_ARRAY();
			static_assert(is_same<C, Component>::value, "Component type not matching");

			using Data::isNull;
			using Super::equals;
			using Super::equalOperator;

			using Super::Super;
			using Super::print;

			constexpr WrapImpl() = default;
			constexpr WrapImpl(const WrapImpl&) = default;
			constexpr WrapImpl(WrapImpl&&) = default;

			bool operator !=(nullptr_t) const noexcept
			{
				return !isNull();
			}
			bool operator ==(nullptr_t) const noexcept
			{
				return isNull();
			}
			friend bool operator !=(nullptr_t, const WrapImpl& _this) noexcept
			{
				return !_this.isNull();
			}
			friend bool operator ==(nullptr_t, const WrapImpl& _this) noexcept
			{
				return _this.isNull();
			}
			bool operator !=(Ref ref) const noexcept
			{
				return !equals(ref);
			}
			bool operator ==(Ref ref) const noexcept
			{
				return equals(ref);
			}
			template <size_t size>
			bool operator !=(const InternalComponent(&ref)[size]) const noexcept
			{
				return !equals(ref);
			}
			template <size_t size>
			bool operator ==(const InternalComponent(&ref)[size]) const noexcept
			{
				return equals(ref);
			}
			template <typename T> WrapImpl& operator << (const T& arrayable) throws(NotEnoughSpaceException)
			{
				print(arrayable);
				return *this;
			}

			WrapImpl& operator =(const WrapImpl & _copy)
			{
				equalOperator(_copy);
				return *this;
			}
			WrapImpl& operator =(WrapImpl && _move) noexcept
			{
				this->~WrapImpl();
				new (this) WrapImpl(move(_move));
				return *this;
			}

			template <size_t _len>
			constexpr WrapImpl(const InternalComponent(&_str)[_len])
				: Super(_str, _len - Data::is_string)
			{
			}
		};
	}
}
