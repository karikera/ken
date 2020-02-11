#pragma once

#include "quaternion.h"

namespace kr
{
	namespace math
	{

		template <bool aligned>
		inline quaternionT<aligned>::quaternionT() noexcept
		{
		}
		template <bool aligned>
		inline quaternionT<aligned>::quaternionT(const vec& v) noexcept
			:vec(v)
		{
		}
		template <bool aligned>
		inline quaternionT<aligned>::quaternionT(const mat& m) noexcept
		{
			static const vec CV_PMMP = { +1.0f, -1.0f, -1.0f, +1.0f };
			static const vec CV_MPMP = { -1.0f, +1.0f, -1.0f, +1.0f };
			static const vec CV_MMPP = { -1.0f, -1.0f, +1.0f, +1.0f };

			vec r0 = m[0];  // (r00, r01, r02, 0)
			vec r1 = m[1];  // (r10, r11, r12, 0)
			vec r2 = m[2];  // (r20, r21, r22, 0)

							// (r00, r00, r00, r00)
			vec r00 = r0.template shuffle<0, 0, 0, 0>();
			// (r11, r11, r11, r11)
			vec r11 = r0.template shuffle<1, 1, 1, 1>();
			// (r22, r22, r22, r22)
			vec r22 = r0.template shuffle<2, 2, 2, 2>();

			// x^2 >= y^2 equivalent to r11 - r00 <= 0
			// (r11 - r00, r11 - r00, r11 - r00, r11 - r00)
			vec r11mr00 = r11 - r00;
			vec x2gey2 = r11mr00 <= (vec&)CV_ZERO;

			// z^2 >= w^2 equivalent to r11 + r00 <= 0
			// (r11 + r00, r11 + r00, r11 + r00, r11 + r00)
			vec r11pr00 = r11 + r00;
			vec z2gew2 = r11pr00 <= (vec&)CV_ZERO;

			// x^2 + y^2 >= z^2 + w^2 equivalent to r22 <= 0
			vec x2py2gez2pw2 = r22 <= (vec&)CV_ZERO;

			// (+r00, -r00, -r00, +r00)
			vec t0 = CV_PMMP * r00;

			// (-r11, +r11, -r11, +r11)
			vec t1 = CV_MPMP * r11;

			// (-r22, -r22, +r22, +r22)
			vec t2 = CV_MMPP * r22;

			// (4*x^2, 4*y^2, 4*z^2, 4*w^2)
			vec x2y2z2w2 = t0 + t1;
			x2y2z2w2 = t2 + x2y2z2w2;
			x2y2z2w2 = x2y2z2w2 + CV_1;

			// (r01, r02, r12, r11)
			t1 = ::shuffle<1, 2, 2, 1>(r0, r1);
			// (r10, r10, r20, r21)
			t0 = ::shuffle<0, 0, 0, 1>(r1, r2);
			// (r10, r20, r21, r10)
			t0 = t0.template shuffle<0, 2, 3, 1>();

			// (4*x*y, 4*x*z, 4*y*z, ?)
			vec xyxzyz = t0 + t1;

			// (r21, r20, r10, r10)
			t1 = ::shuffle<1, 0, 0, 0>(r2, r1);
			// (r12, r12, r02, r01)
			t0 = ::shuffle<2, 2, 2, 1>(r1, r0);
			// (r12, r02, r01, r12)
			t0 = t0.template shuffle<0, 2, 3, 1>();

			// (4*x*w, 4*y*w, 4*z*w, ?)
			vec xwywzw = t0 - t1;
			xwywzw = CV_MPMP * xwywzw;

			// (4*x^2, 4*y^2, 4*x*y, ?)
			t0 = ::shuffle<0, 1, 0, 0>(x2y2z2w2, xyxzyz);
			// (4*z^2, 4*w^2, 4*z*w, ?)
			t1 = ::shuffle<2, 3, 2, 0>(x2y2z2w2, xwywzw);
			// (4*x*z, 4*y*z, 4*x*w, 4*y*w)
			t2 = ::shuffle<1, 2, 0, 1>(xyxzyz, xwywzw);

			// (4*x*x, 4*x*y, 4*x*z, 4*x*w)
			vec tensor0 = ::shuffle<0, 2, 0, 2>(t0, t2);
			// (4*y*x, 4*y*y, 4*y*z, 4*y*w)
			vec tensor1 = ::shuffle<2, 1, 1, 3>(t0, t2);
			// (4*z*x, 4*z*y, 4*z*z, 4*z*w)
			vec tensor2 = ::shuffle<0, 1, 0, 2>(t2, t1);
			// (4*w*x, 4*w*y, 4*w*z, 4*w*w)
			vec tensor3 = ::shuffle<2, 3, 2, 1>(t2, t1);

			// Select the row of the tensor-product matrix that has the largest
			// magnitude.
			t0 = (x2gey2 & tensor0) | andnot(x2gey2, tensor1);
			t1 = (z2gew2 & tensor2) | andnot(z2gew2, tensor3);
			t2 = (x2py2gez2pw2 & t0) | andnot(x2py2gez2pw2, t1);

			// Normalize the row.  No division by zero is possible because the
			// quaternionT is unit-length (and the row is a nonzero multiple of
			// the quaternionT).
			*this = t2.normalize();
		}
		template <bool aligned>
		inline quaternionT<aligned>::quaternionT(float x, float y, float z, float w) noexcept
			: vec(x, y, z, w)
		{
		}
		template <bool aligned>
		inline const quaternionT<aligned> quaternionT<aligned>::rotateX(float rad) noexcept
		{
			SinCos sc = math::sincos(rad / 2.f);
			return quaternionT(sc.sin, 0, 0, sc.cos);
		}
		template <bool aligned>
		inline const quaternionT<aligned> quaternionT<aligned>::rotateY(float rad) noexcept
		{
			SinCos sc = math::sincos(rad / 2.f);
			return quaternionT(0, sc.sin, 0, sc.cos);
		}
		template <bool aligned>
		inline const quaternionT<aligned> quaternionT<aligned>::rotateZ(float rad) noexcept
		{
			SinCos sc = math::sincos(rad / 2.f);
			return quaternionT(0, 0, sc.sin, sc.cos);
		}
		template <bool aligned>
		inline const quaternionT<aligned> quaternionT<aligned>::rotateAxis(const vec& v, float rad) noexcept
		{
			vec s, c;
			sincosv(&s, &c, vec(0.5f * rad));
			return ((v * s) & (vec&)CV_MASK_XYZ) | (c & (vec&)CV_MASK_W);
		}

		template <bool aligned>
		inline const kr::mat4a quaternionT<aligned>::toMatrix() const noexcept
		{
			struct
			{
				union
				{
					vec v1;
					struct { float xx, yy, zz, ww; };
				};
				union
				{
					vec v2;
					struct { float xy, yz, wz, wx; };
				};
				union
				{
					vec v3;
					struct { float xz, wy, _t1, _t2; };
				};
			} v;
			v.v1 = *(vec*)this * *(vec*)this;
			v.v2 = *(vec*)this * this->template shuffle<1, 2, 3, 0>();
			v.v3 = *(vec*)this * this->template shuffle<2, 3, 0, 0>();

			mat m;
			float* o = (float*)(&m);
			o[0] = v.ww + v.xx - v.yy - v.zz;	o[1] = 2 * (v.xy - v.wz);			o[2] = 2 * (v.xz + v.wy);			o[3] = 0;
			o[4] = 2 * (v.xy + v.wz);			o[5] = v.ww - v.xx + v.yy - v.zz;   o[6] = 2 * (v.yz - v.wx);			o[7] = 0;
			o[8] = 2 * (v.xz - v.wy);			o[9] = 2 * (v.yz + v.wx);			o[10] = v.ww - v.xx - v.yy + v.zz;  o[11] = 0;
			o[12] = 0;							o[13] = 0;							o[14] = 0;							o[15] = 1;
			return m;
		}
		template <bool aligned>
		inline quaternionT<aligned>& quaternionT<aligned>::operator *=(const quaternionT& q) noexcept
		{
			return *this = *this * q;
		}
		template <bool aligned>
		inline const quaternionT<aligned> quaternionT<aligned>::operator *(const quaternionT& q) const noexcept
		{
			const vec& a = *this;
			const vec& b = q;

			static const vec ControlWZYX = { 1.0f,-1.0f, 1.0f,-1.0f };
			static const vec ControlZWXY = { 1.0f, 1.0f,-1.0f,-1.0f };
			static const vec ControlYXWZ = { -1.0f, 1.0f, 1.0f,-1.0f };
			vec Q2X = b;
			vec Q2Y = b;
			vec Q2Z = b;
			vec vResult = b;
			vResult = vResult.template shuffle<3, 3, 3, 3>();
			Q2X = Q2X.template shuffle<0, 0, 0, 0>();
			Q2Y = Q2Y.template shuffle<1, 1, 1, 1>();
			Q2Z = Q2Z.template shuffle<2, 2, 2, 2>();
			vResult *= a;

			vec Q1Shuffle = a;
			Q1Shuffle = Q1Shuffle.template shuffle<3, 2, 1, 0>();
			Q2X *= Q1Shuffle;
			Q1Shuffle = Q1Shuffle.template shuffle<1, 0, 3, 2>();
			Q2X *= ControlWZYX;

			Q2Y *= Q1Shuffle;
			Q1Shuffle = Q1Shuffle.template shuffle<3, 2, 1, 0>();
			Q2Y *= ControlZWXY;
			Q2Z *= Q1Shuffle;
			vResult += Q2X;
			Q2Y += Q2Z * ControlYXWZ;
			vResult += Q2Y;
			return vResult;
		}
	}
}

template <bool aligned>
inline const kr::math::quaternionT<aligned> slerp(const kr::math::quaternionT<aligned>& a, const kr::math::quaternionT<aligned>& b, const kr::math::vector<float, 4, aligned>& rate) noexcept
{
	using vec = kr::math::vector<float, 4, aligned>;
	return slerpv((vec&)a, (vec&)b, rate);
}
template <bool aligned>
inline const kr::math::quaternionT<aligned> slerp(const kr::math::quaternionT<aligned>& a, const kr::math::quaternionT<aligned>& b, float rate) noexcept
{
	using vec = kr::math::vector<float, 4, aligned>;
	return slerp(a, b, (vec)rate);
}