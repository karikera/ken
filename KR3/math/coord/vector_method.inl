#pragma once
#include "vector_method.h"
#ifdef WIN32
#include "mmx.h"
#endif

namespace kr
{
	namespace math
	{
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 2, aligned, order>::rotate_xy() const noexcept
		{
			vector<T, fullcount, aligned, order> v = ((vector<T, fullcount, aligned, order>*)this)->template shuffle<1, 0>();
			v.x = -v.x;
			return v;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 2, aligned, order>::rotate_yx() const noexcept
		{
			vector<T, fullcount, aligned, order> v = ((vector<T, fullcount, aligned, order>*)this)->template shuffle<1, 0>();
			v.y = -v.y;
			return v;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 2, aligned, order>::rotate_z(T rad) const noexcept
		{
			SinCosT<T> sc = math::sincos_t(rad);

			vector<T, fullcount, aligned, order> out = *(vector<T, fullcount, aligned, order>*)this;
			out.x = (T)(out.x * sc.cos - out.y * sc.sin);
			out.y = (T)(out.x * sc.sin + out.y * sc.cos);
			return out;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 2, aligned, order>::direction_z(T angle) noexcept
		{
			SinCosT<T> out = math::sincos_t(angle);
			return {out.cos, out.sin};
		}

		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 3, aligned, order>::rotate_xz() const noexcept
		{
			vector<T, fullcount, aligned, order> v = ((vector<T, fullcount, aligned, order>*)this)->template shuffle<2, 1, 0>();
			v.x = -v.x;
			return v;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 3, aligned, order>::rotate_zx() const noexcept
		{
			vector<T, fullcount, aligned, order> v = ((vector<T, fullcount, aligned, order>*)this)->template shuffle<2, 1, 0>();
			v.z = -v.z;
			return v;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 3, aligned, order>::rotate_yz() const noexcept
		{
			vector<T, fullcount, aligned, order> v = ((vector<T, fullcount, aligned, order>*)this)->template shuffle<0, 2, 1>();
			v.y = -v.y;
			return v;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 3, aligned, order>::rotate_zy() const noexcept
		{
			vector<T, fullcount, aligned, order> v = ((vector<T, fullcount, aligned, order>*)this)->template shuffle<0, 2, 1>();
			v.z = -v.z;
			return v;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 3, aligned, order>::rotate_x(T rad) const noexcept
		{
			SinCosT<T> sc = math::sincos_t(rad);

			vector<T, fullcount, aligned, order> out = *(vector<T, fullcount, aligned, order>*)this;
			out.y = (T)(out.y * sc.cos - out.z * sc.sin);
			out.z = (T)(out.y * sc.sin + out.z * sc.cos);
			return out;
		}
		template <typename T, size_t fullcount, bool aligned, typename order>
		ATTR_INLINE const vector<T, fullcount, aligned, order> _pri_::vector_method<T, fullcount, 3, aligned, order>::rotate_y(T rad) const noexcept
		{
			SinCosT<T> sc = sincos_t(rad);

			vector<T, fullcount, aligned, order> out = *(vector<T, fullcount, aligned, order>*)this;
			out.z = (T)(out.z * sc.cos - out.x * sc.sin);
			out.x = (T)(out.z * sc.sin + out.x * sc.cos);
			return out;
		}

		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 2, true, OrderRGBA>::rotate_xy() const noexcept
		{
			return ((vec4a*)this)->shuffle<1, 0>() * CV_MINUS_X;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 2, true, OrderRGBA>::rotate_yx() const noexcept
		{
			return ((vec4a*)this)->shuffle<1, 0>() * CV_MINUS_Y;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 3, true, OrderRGBA>::rotate_xz() const noexcept
		{
			return ((vec4a*)this)->shuffle<2, 1, 0>() * CV_MINUS_X;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 3, true, OrderRGBA>::rotate_zx() const noexcept
		{
			return ((vec4a*)this)->shuffle<2, 1, 0>() * CV_MINUS_Z;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 3, true, OrderRGBA>::rotate_yz() const noexcept
		{
			return ((vec4a*)this)->shuffle<0, 2, 1>() * CV_MINUS_Y;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 3, true, OrderRGBA>::rotate_zy() const noexcept
		{
			return ((vec4a*)this)->shuffle<0, 2, 1>() * CV_MINUS_Z;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 3, true, OrderRGBA>::rotate_x(float rad) const noexcept
		{
			SinCos angle = sincos(rad);

			vec4a cs = { 1, angle.cos, angle.sin, 0 };
			const vec4a &_this = *(vec4a*)this;
			vec4a out = _this.shuffle<0, 1, 1, 3>() * cs;
			out += _this.shuffle<0, 2, 2, 3>() * cs.shuffle<3, 2, 1, 0>() * CV_MINUS_Y;
			return out;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 3, true, OrderRGBA>::rotate_y(float rad) const noexcept
		{
			SinCos angle = sincos(rad);

			vec4a cs = { angle.sin, 1, angle.cos, 0 };
			const vec4a &_this = *(vec4a*)this;
			vec4a out = _this.shuffle<2, 1, 2, 3>() * cs;
			out += _this.shuffle<0, 1, 0, 3>() * cs.shuffle<2, 3, 0, 1>() * CV_MINUS_Z;
			return out;
		}
		template <>
		ATTR_INLINE const vec4a _pri_::vector_method<float, 4, 2, true, OrderRGBA>::rotate_z(float rad) const noexcept
		{
			SinCos angle = sincos(rad);

			vec4a cs = { angle.cos, angle.sin, 1, 0 };
			const vec4a &_this = *(vec4a*)this;
			vec4a out = _this.shuffle<0, 0, 2, 3>() * cs;
			out += _this.shuffle<1, 1, 2, 3>() * cs.shuffle<1, 0, 3, 2>() * CV_MINUS_X;
			return out;
		}
	}
}