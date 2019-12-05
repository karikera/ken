#pragma once


#include "type.h"

namespace kr
{
	class undefined_t final
	{
	};
	static constexpr const undefined_t undefined = undefined_t();

	namespace _pri_
	{
		class UndefinedWrite :public Bufferable<UndefinedWrite, BufferInfo<AutoComponent>>
		{
		public:
			inline UndefinedWrite(undefined_t) noexcept
			{
			}
			template <typename T>
			inline size_t $sizeAs() const noexcept
			{
				return 9;
			}
			template <typename T>
			inline size_t $copyTo(T* dest) const noexcept
			{
				*dest++ = 'u';
				*dest++ = 'n';
				*dest++ = 'd';
				*dest++ = 'e';
				*dest++ = 'f';
				*dest++ = 'i';
				*dest++ = 'n';
				*dest++ = 'e';
				*dest++ = 'd';
				return 9;
			}
		};
	}

	template <typename C> struct Bufferize<undefined_t, C> { using type = _pri_::UndefinedWrite; };
}
