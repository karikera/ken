#pragma once

#include "../main.h"
#include "if.h"

namespace kr
{
	
	namespace meta
	{
		// maximum
		template <int v, int ... values> struct maximum
		{
			using super = maximum<values ...>;
			static constexpr int value = super::value > v ? super::value : v;
		};;
		template <int v> struct maximum<v>
		{
			static constexpr int value = v;
		};;

		// lastbit
		template <typename T> struct LastBitOf
		{
			static constexpr size_t value = 1 << (sizeof(T)*8-1);
		};
	
		// max of / over of
		namespace _pri_
		{
			template <typename T>
			struct UnsignedNumberInfo
			{
				static constexpr T maxof = (T)-1;
				static constexpr T minof = 0;
				static constexpr float overof = (float)((T)1 << (sizeof(T) * 8 - 1)) * 2.f;
			};
			template <typename T>
			struct SignedNumberInfo
			{
				static constexpr T maxof = (T)((std::make_unsigned_t<T>)((T)-1) >> 1);
				static constexpr T minof = - maxof - 1;
				static constexpr float overof = (float)((((std::make_unsigned_t<T>)-1) >> 1) + 1);
			};
			template <typename T>
			struct FloatingNumberInfo
			{
				static constexpr float maxof = INFINITY;
				static constexpr float minof = -INFINITY;
				static constexpr float overof = INFINITY;
			};
		}
		template <typename T> struct NumberInfo:
			meta::if_t<
				std::is_unsigned_v<T>,
				_pri_::UnsignedNumberInfo<T>,
				meta::if_t<
					std::is_floating_point_v<T>,
					_pri_::FloatingNumberInfo<T>,
					_pri_::SignedNumberInfo<T>
				>
			>
		{
		};
	}


	template <typename TYPE, size_t alignment>
	struct alignedsize
	{
		static constexpr size_t value = (sizeof(TYPE) + alignment - 1) / alignment * alignment;
	};
}

#define lastbitof(type) (::kr::meta::LastBitOf<type>::value)
#define maxof(type)	(::kr::meta::NumberInfo<type>::maxof)
#define minof(type)	(::kr::meta::NumberInfo<type>::minof)
#define overof(type) (::kr::meta::NumberInfo<type>::overof)