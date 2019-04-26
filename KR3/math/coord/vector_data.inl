#include "vector_data.h"
#include "vector_data.h"
#pragma once

#include "vector_data.h"


namespace kr
{
	namespace math
	{
		using vec4a_ctor = vector_constructor<float, 0, true, OrderRGBA, float, float, float, float>;
		using ivec4a_ctor = vector_constructor<int, 0, true, OrderRGBA, int, int, int, int>;

		template <typename T, size_t count, bool aligned, typename order>
		ATTR_INLINE vector_base<T, count, aligned, order>::vector_base(T arg) noexcept
		{
			for (T & dest : m_data)
				dest = arg;
		}

		template <bool aligned, typename order>
		ATTR_INLINE vector_constructor<byte, 0, aligned, order, byte, byte, byte, byte>::vector_constructor(byte _r, byte _g, byte _b, byte _a) noexcept
		{
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		}
		template <bool aligned, typename order>
		ATTR_INLINE vector_constructor<byte, 0, aligned, order, byte, byte, byte, byte>::vector_constructor(dword _value) noexcept
		{
			value = _value;
		}

		ATTR_INLINE vec4a_ctor::vector_constructor(float _all) noexcept
		{
#if defined(EMSIMD) || defined(MSSIMD)
			m_vector = _mm_set_ps1(_all);
#else
			m_vector.x = m_vector.y = m_vector.z = m_vector.w = _all;
#endif
		}
		ATTR_INLINE vec4a_ctor::vector_constructor(const __vector4 & _v) noexcept
		{
			m_vector = _v;
		}
		ATTR_INLINE vec4a_ctor::vector_constructor(float _x, float _y, float _z, float _w) noexcept
		{
#if defined(EMSIMD) || defined(MSSIMD)
			m_vector = _mm_set_ps(_w, _z, _y, _x);
#else
			m_vector.x = _x;
			m_vector.y = _y;
			m_vector.z = _z;
			m_vector.w = _w;
#endif
		}
		ATTR_INLINE vec4a_ctor::vector_constructor(const ivec4a &_v) noexcept
		{
#ifdef EMSIMD
			m_vector = (__vector4)_v.m_vector;
#elif defined(MSSIMD)
			m_vector = _mm_cvtepi32_ps(_v.m_vector);
#else
			m_vector.x = (float)_v.x;
			m_vector.y = (float)_v.y;
			m_vector.z = (float)_v.z;
			m_vector.w = (float)_v.w;
#endif
		}
		ATTR_INLINE const ivec4a vec4a_ctor::ibits() const noexcept
		{
#ifdef EMSIMD
			return (__ivector4&)(m_vector);
#elif defined(MSSIMD)
			return _mm_castps_si128(m_vector);
#else
			return ivec4a{
				(int&)x,
				(int&)y,
				(int&)z,
				(int&)w
			};
#endif
		}
		ATTR_INLINE const ivec4a vec4a_ctor::itruncate() const noexcept
		{
#ifdef MSSIMD
			return _mm_cvttps_epi32(m_vector);
#else
			union {
				int x[4];
				__ivector4 m;
			} u;
			for (int i = 0; i < 4; ++i)
			{
				int x = lrint(m_vector[i]);
				if (x != 0 || fabs(m_vector[i]) < 2.0)
					u.x[i] = (int)m_vector[i];
				else
					u.x[i] = (int)0x80000000;
			}
			return u.m;
#endif
		}
		ATTR_INLINE const ivec4a vec4a_ctor::iround() const noexcept
		{
#ifdef EMSIMD
			return emscripten_int32x4_fromFloat32x4(m_vector);
#elif defined(MSSIMD)
			return _mm_cvtps_epi32(m_vector);
#else
			ivec4a out;
			out.m_vector[0] = lroundf(m_vector[0]);
			out.m_vector[1] = lroundf(m_vector[1]);
			out.m_vector[2] = lroundf(m_vector[2]);
			out.m_vector[3] = lroundf(m_vector[3]);
			return out;
#endif
		}
		ATTR_INLINE float vec4a_ctor::getX() const noexcept
		{
#if defined(EMSIMD) || defined(MSSIMD)
			return _mm_cvtss_f32(m_vector);
#else
			return m_vector.x;
#endif
		}
		ATTR_INLINE float vec4a_ctor::getY() const noexcept
		{
#ifdef USE_SIMD
			return getYV().getX();
#else
			return m_vector.y;
#endif
		}
		ATTR_INLINE float vec4a_ctor::getZ() const noexcept
		{
#ifdef USE_SIMD
			return getZV().getX();
#else
			return m_vector.z;
#endif
		}
		ATTR_INLINE float vec4a_ctor::getW() const noexcept
		{
#ifdef USE_SIMD
			return getWV().getX();
#else
			return m_vector.w;
#endif
		}
		ATTR_INLINE const vec4a vec4a_ctor::getXV() const noexcept
		{
			return ((vec4a*)this)->shuffle<0, 0, 0, 0>();
		}
		ATTR_INLINE const vec4a vec4a_ctor::getYV() const noexcept
		{
			return ((vec4a*)this)->shuffle<1, 1, 1, 1>();
		}
		ATTR_INLINE const vec4a vec4a_ctor::getZV() const noexcept
		{
			return ((vec4a*)this)->shuffle<2, 2, 2, 2>();
		}
		ATTR_INLINE const vec4a vec4a_ctor::getWV() const noexcept
		{
			return ((vec4a*)this)->shuffle<3, 3, 3, 3>();
		}
		ATTR_INLINE const vec4a vec4a_ctor::changeX(float _v) const noexcept
		{
			const vec4a &vec4this = *static_cast<const vec4a*>(this);
			return vec4this.movs(vec4a::makes(_v));
		}
		ATTR_INLINE const vec4a vec4a_ctor::changeY(float _v) const noexcept
		{
			const vec4a &vec4this = *static_cast<const vec4a*>(this);
			vec4a res = vec4this.shuffle<1, 0, 2, 3>();
			res = res.changeX(_v);
			return res.shuffle<1, 0, 2, 3>();
		}
		ATTR_INLINE const vec4a vec4a_ctor::changeZ(float _v) const noexcept
		{
			const vec4a &vec4this = *static_cast<const vec4a*>(this);
			vec4a res = vec4this.shuffle<2, 1, 0, 3>();
			res = res.changeX(_v);
			return res.shuffle<2, 1, 0, 3>();
		}
		ATTR_INLINE const vec4a vec4a_ctor::changeW(float _v) const noexcept
		{
			const vec4a &vec4this = *static_cast<const vec4a*>(this);
			vec4a res = vec4this.shuffle<3, 1, 2, 0>();
			res = res.changeX(_v);
			return res.shuffle<3, 1, 2, 0>();
		}

		ATTR_INLINE ivec4a_ctor::vector_constructor(int _all) noexcept
		{
#ifdef __EMSCRIPTEN__
			m_vector = __ivector4{ _all, _all, _all, _all };
#else
			m_vector = _mm_set1_epi32(_all);
#endif
		}
		ATTR_INLINE ivec4a_ctor::vector_constructor(const __ivector4 & _v) noexcept
		{
			m_vector = _v;
		}
		ATTR_INLINE ivec4a_ctor::vector_constructor(int _x, int _y, int _z, int _w) noexcept
		{
#ifdef MSSIMD
			m_vector = _mm_set_epi32(_w, _z, _y, _x);
#else
			m_vector = __ivector4{ _w, _z, _y, _x };
#endif
		}
		ATTR_INLINE const vec4a ivec4a_ctor::fbits() const noexcept
		{
#ifdef MSSIMD
			return _mm_castsi128_ps(m_vector);
#else
			return (__vector4&)m_vector;
#endif
		}
		ATTR_INLINE int ivec4a_ctor::getX() const noexcept
		{
#ifdef MSSIMD
			return _mm_cvtsi128_si32(m_vector);
#else
			return m_vector[0];
#endif
		}
		ATTR_INLINE int ivec4a_ctor::getY() const noexcept
		{
#ifdef MSSIMD
			return getYV().getX();
#else
			return m_vector[1];
#endif
		}
		ATTR_INLINE int ivec4a_ctor::getZ() const noexcept
		{
#ifdef MSSIMD
			return getZV().getX();
#else
			return m_vector[2];
#endif
		}
		ATTR_INLINE int ivec4a_ctor::getW() const noexcept
		{
#ifdef MSSIMD
			return getWV().getX();
#else
			return m_vector[3];
#endif
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::getXV() const noexcept
		{
			return ((ivec4a*)this)->shuffle<0, 0, 0, 0>();
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::getYV() const noexcept
		{
			return ((ivec4a*)this)->shuffle<1, 1, 1, 1>();
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::getZV() const noexcept
		{
			return ((ivec4a*)this)->shuffle<2, 2, 2, 2>();
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::getWV() const noexcept
		{
			return ((ivec4a*)this)->shuffle<3, 3, 3, 3>();
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::changeX(int _v) const noexcept
		{
			const ivec4a &ivec4this = *static_cast<const ivec4a*>(this);
			return (ivec4this & CV_MASK_YZW) | ivec4a::makes(_v);
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::changeY(int _v) const noexcept
		{
			const ivec4a &ivec4this = *static_cast<const ivec4a*>(this);
			return (ivec4this & CV_MASK_XZW) | ivec4a::makes(_v).shuffle<1, 0, 2, 3>();
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::changeZ(int _v) const noexcept
		{
			const ivec4a &ivec4this = *static_cast<const ivec4a*>(this);
			return (ivec4this & CV_MASK_XYW) | ivec4a::makes(_v).shuffle<2, 1, 0, 3>();
		}
		ATTR_INLINE const ivec4a ivec4a_ctor::changeW(int _v) const noexcept
		{
			const ivec4a &ivec4this = *static_cast<const ivec4a*>(this);
			return (ivec4this & CV_MASK_XYZ) | ivec4a::makes(_v).shuffle<3, 1, 2, 0>();
		}

	}
}
