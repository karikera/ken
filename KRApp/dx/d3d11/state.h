#pragma once

#include "common.h"
#include "device.h"

namespace kr
{
	namespace d3d11
	{
		template <typename T>
		struct StateConst;

		class StateIndex
		{
		public:
			template <typename T> int make(T value);

			StateIndex() noexcept;

			template <typename T>
			StateIndex(T value) noexcept;

			template <typename T>
			StateIndex& operator |=(T value) noexcept;

			template <typename T>
			const StateIndex operator |(T value) const noexcept;

			template <typename T>
			friend const StateIndex operator |(T value, const StateIndex im) noexcept
			{
				im |= value;
				return im;
			}

			operator int() const noexcept;

		private:
			int index;
			int maximum;

		};

		template <typename T>
		StateIndex::StateIndex(T value) noexcept
		{
			index = make(value);
			maximum = StateConst<T>::count;
		}

		template <typename T>
		StateIndex& StateIndex::operator |=(T value) noexcept
		{
			typedef StateConst<T> C;
			index += make(value) * maximum;
			maximum *= C::count;
			return *this;
		}

		template <typename T>
		const StateIndex StateIndex::operator |(T value) const noexcept
		{
			StateIndex out = *this;
			out |= value;
			return out;
		}

		template <typename T> int StateIndex::make(T value)
		{
			typedef StateConst<T> C;
			int res = (int)value - C::offset;
			assert(res < C::count);
			return res;
		}

		template <>
		int StateIndex::make<Filter>(Filter value);
	}
}

