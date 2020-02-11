#pragma once

#include "matrix.h"

namespace kr
{
	namespace math
	{
		template <bool aligned>
		class quaternionT:private vector<float, 4, aligned, OrderRGBA>
		{
		private:
			using vec = vector<float, 4, aligned, OrderRGBA>;
			using mat = matrix<float, 4, 4, aligned>;
			using vec::shuffle;

		public:
			using vec::x;
			using vec::y;
			using vec::z;
			using vec::w;
			using vec::make;
			using vec::normalize;

			inline quaternionT() noexcept;
			inline quaternionT(const vec& v) noexcept;
			inline quaternionT(const mat& m) noexcept;
			inline quaternionT(float x, float y, float z, float w) noexcept;
			inline static const quaternionT rotateX(float rad) noexcept;
			inline static const quaternionT rotateY(float rad) noexcept;
			inline static const quaternionT rotateZ(float rad) noexcept;
			inline static const quaternionT rotateAxis(const vec &v, float rad) noexcept;
			inline const mat4a toMatrix() const noexcept;

			inline quaternionT & operator *=(const quaternionT & q) noexcept;
			inline const quaternionT operator *(const quaternionT & q) const noexcept;
		};

	}

}


template <bool aligned>
inline const kr::math::quaternionT<aligned> slerp(const kr::math::quaternionT<aligned>& a, const kr::math::quaternionT<aligned>& b, const kr::math::vector<float, 4, aligned>& rate) noexcept;
template <bool aligned>
inline const kr::math::quaternionT<aligned> slerp(const kr::math::quaternionT<aligned>& a, const kr::math::quaternionT<aligned>& b, float rate) noexcept;
