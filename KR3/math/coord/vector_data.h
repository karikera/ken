#pragma once

#include "../../main.h"
#include "../../meta/numbers.h"

#ifndef __EMSCRIPTEN__
#define USE_SIMD
#endif

#ifdef USE_SIMD
#ifdef __EMSCRIPTEN__
#define EMSIMD
#include "mmx.h"
#define _mm_shuffle_epi32(a, mask) \
  ((__ivector4)__builtin_shufflevector((__ivector4)(a), (__ivector4)(a), \
	(((mask) >> 0) & 0x3) + 0, \
	(((mask) >> 2) & 0x3) + 0, \
	(((mask) >> 4) & 0x3) + 4, \
	(((mask) >> 6) & 0x3) + 4))

#elif defined(_MSC_VER)
#define MSSIMD
#include "mmx.h"
#else
#define NOSIMD
#define USE_SIMD
#endif
#endif

namespace kr
{
	namespace math
	{
#ifdef EMSIMD
		using __vector4 = float __attribute__((ext_vector_type(4)));
		using __ivector4 = int __attribute__((ext_vector_type(4)));

#elif defined(MSSIMD)
#include <emmintrin.h>
#include <smmintrin.h>
		using __vector4 = __m128;
		using __ivector4 = __m128i;
#else
		struct __vector4 {
			float x, y, z, w;
			inline float & operator [](size_t idx) { return ((float*)this)[idx]; };
			inline const float & operator [](size_t idx) const { return ((float*)this)[idx]; };
		};
		struct __ivector4 {
			int x, y, z, w;
			inline int & operator [](size_t idx) { return ((int*)this)[idx]; };
			inline const int & operator [](size_t idx) const { return ((int*)this)[idx]; };
		};
#endif

		template <typename T, size_t size, bool aligned, typename order>
		class vector_data
		{
		public:
			union
			{
				T m_data[size];
			};
		};
		template <typename T, bool aligned, typename order>
		class vector_data<T, 0, aligned, order>;
		template <typename T, bool aligned, typename order>
		class vector_data<T, 1, aligned, order>;
		template <typename T, bool aligned>
		class vector_data<T, 2, aligned, OrderRGBA>
		{
		public:
			union
			{
				struct { T m_data[2]; };
				struct { T x, y; };
				struct { T r, g; };
				struct { T u, v; };
				struct { T m_vector[2]; };
			};
		};
		template <typename T, bool aligned>
		class vector_data<T, 3, aligned, OrderRGBA>
		{
		public:
			union
			{
				struct { T m_data[3]; };
				struct { T x, y, z; };
				struct { T r, g, b; };
				struct { T u, v, w; };
				struct { T m_vector[3]; };
			};
		};
		template <typename T, bool aligned>
		class vector_data<T, 4, aligned, OrderRGBA>
		{
		public:
			union
			{
				struct { T m_data[4]; };
				struct { T x, y, z, w; };
				struct { T r, g, b, a; };
				struct { T m_vector[4]; };
			};
		};
		template <typename T, bool aligned>
		class vector_data<T, 4, aligned, OrderBGRA>
		{
		public:
			union
			{
				struct { T m_data[4]; };
				struct { T z, y, x, w; };
				struct { T b, g, r, a; };
				struct { T m_vector[4]; };
			};
		};
		template <bool aligned>
		struct vector_data<byte, 4, aligned, OrderRGBA>
		{
			union
			{
				struct { byte m_data[4]; };
				struct { byte x, y, z, w; };
				struct { byte r, g, b, a; };
				dword value;
				struct { byte m_vector[4]; };
			};
		};
		template <bool aligned>
		struct vector_data<byte, 4, aligned, OrderBGRA>
		{
			union
			{
				struct { byte m_data[4]; };
				struct { byte z, y, x, w; };
				struct { byte b, g, r, a; };
				dword value;
				struct { byte m_vector[4]; };
			};
		};
		template <> 
		struct vector_data<float, 4, true, OrderRGBA>
		{
			union
			{
				struct { float m_data[4]; };
				struct { float x, y, z, w; };
				struct { float r, g, b, a; };
				__vector4 m_vector;
			};
		};
		template <>
		struct vector_data<int, 4, true, OrderRGBA>
		{
			union
			{
				struct { int m_data[4]; };
				struct { int x, y, z, w; };
				struct { int r, g, b, a; };
				__ivector4 m_vector;
			};
		};

		template <typename T, size_t count, bool aligned, typename order>
		class vector_base
			: public vector_data<T, count, aligned, order>
		{
		public:
			using vector_data<T, count, aligned, order>::m_data;
			using vector_data<T, count, aligned, order>::vector_data;

			vector_base() = default;
			explicit vector_base(T arg) noexcept;
		};

		template <typename T, size_t count, bool aligned, typename order, typename ... params>
		class vector_constructor
			:public vector_constructor<T, count - 1, aligned, order, T, params ...>
		{
		public:
			using vector_constructor<T, count - 1, aligned, order, T, params ...>::vector_constructor;
			using vector_constructor<T, count - 1, aligned, order, T, params ...>::m_data;

			vector_constructor() = default;
			
			template <bool aligned2>
			ATTR_INLINE explicit vector_constructor(const vector<T, count, aligned2, order> & o, params ... ext) noexcept
			{
				memcpy(m_data, o.m_data, count * sizeof(T));
				T list[] = { ext ... };
				memcpy(m_data + count, list, sizeof ... (params) * sizeof(T));
			}
		};

		template <typename T, bool aligned, typename order, typename ... params>
		class vector_constructor<T, 1, aligned, order, params ...> 
			:public vector_constructor<T, 0, aligned, order, T, params ...>
		{
		public:
			using vector_constructor<T, 0, aligned, order, T, params ...>::vector_constructor;
			using vector_constructor<T, 0, aligned, order, T, params ...>::m_data;
		};

		template <typename T, size_t count, bool aligned, typename order>
		class vector_constructor<T, count, aligned, order> 
			:public vector_constructor<T, count - 1, aligned, order, T>
		{
		public:
			using vector_constructor<T, count - 1, aligned, order, T>::vector_constructor;
			using vector_constructor<T, count - 1, aligned, order, T>::m_data;
		};
			
		template <typename T, bool aligned, typename order, typename ... params>
		class vector_constructor<T, 0, aligned, order, params ...> 
			:public vector_base<T, sizeof ... (params), aligned, order>
		{
			using super = vector_base<T, sizeof ... (params), aligned, order>;
		public:
			using constructor_t = vector_constructor;
			using super::m_data;
			using super::super;

			vector_constructor() = default;
			vector_constructor(const vector_constructor&) = default;
#pragma warning(push)
#pragma warning(disable:26495)
			ATTR_INLINE vector_constructor(params ... args) noexcept
			{
				const T buffer[] = { args ... };
				meta::order_util<order, OrderBasic, sizeof ... (params)>::copy(m_data, buffer);
			}
#pragma warning(pop)
			template <typename T2, size_t count2, bool aligned2, typename order2>
			ATTR_INLINE explicit vector_constructor(const vector_data<T2, count2, aligned2, order2> &v) noexcept
			{
				static_assert(count2 >= sizeof ... (params), "Need extra parameters");
				meta::order_util<order, order2, sizeof ...(params)>::copy(m_data, v.m_data);
			}
			template <size_t count2, bool aligned2>
			ATTR_INLINE explicit vector_constructor(const vector_data<T, count2, aligned2, order> &v) noexcept
			{
				static_assert(count2 >= sizeof ... (params), "Need extra parameters");
				memcpy(m_data, v.m_data, sizeof(*this));
			}
		};

		template <bool aligned, typename order>
		class vector_constructor<byte, 0, aligned, order, byte, byte, byte, byte> 
			:public vector_base<byte, 4, aligned, order>
		{
			using super = vector_base<byte, 4, aligned, order>;
		public:
			using constructor_t = vector_constructor;
			using super::m_data;
			using super::value;
			using super::r;
			using super::g;
			using super::b;
			using super::a;

			vector_constructor() = default;
			vector_constructor(const vector_constructor&) = default;
			ATTR_INLINE vector_constructor(byte _r, byte _g, byte _b, byte _a) noexcept;
			ATTR_INLINE explicit vector_constructor(dword _value) noexcept;
			template <typename T2, size_t count2, bool aligned2, typename order2>
			ATTR_INLINE explicit vector_constructor(const vector_data<T2, count2, aligned2, order2> &v) noexcept
			{
				static_assert(count2 >= 4, "Need extra parameters");
				meta::order_util<order, order2, 4>::copy(m_data, v.m_data);
			}
			template <size_t count2, bool aligned2>
			ATTR_INLINE explicit vector_constructor(const vector_data<byte, count2, aligned2, order> &v) noexcept
			{
				static_assert(count2 >= 4, "Need extra parameters");
				memcpy(m_data, v.m_data, sizeof(*this));
			}
		};

		template <> 
		class vector_constructor<float, 0, true, OrderRGBA, float, float, float, float>
			:public vector_base<float, 4, true, OrderRGBA>
		{
		public:
			using constructor_t = vector_constructor;
			using vector_base<float, 4, true, OrderRGBA>::m_data;
			using vector_base<float, 4, true, OrderRGBA>::vector_base;
			vector_constructor() = default;
			vector_constructor(const vector_constructor&) = default;
			ATTR_INLINE explicit vector_constructor(float _all) noexcept;
			ATTR_INLINE vector_constructor(const __vector4& v) noexcept;
			ATTR_INLINE vector_constructor(float _x, float _y, float _z, float _w = 1.f) noexcept;
			ATTR_INLINE explicit vector_constructor(const ivec4a &v) noexcept;
			ATTR_INLINE const ivec4a ibits() const noexcept;
			ATTR_INLINE const ivec4a itruncate() const noexcept;
			ATTR_INLINE const ivec4a iround() const noexcept;
			template <typename T2, size_t count2, bool aligned2, typename order2>
			ATTR_INLINE explicit vector_constructor(const vector_data<T2, count2, aligned2, order2> &v) noexcept
			{
				static_assert(count2 >= 4, "Need extra parameters");
				meta::order_util<OrderRGBA, order2, 4>::copy(m_data, v.m_data);
			}
			template <size_t count2, bool aligned2>
			ATTR_INLINE explicit vector_constructor(const vector_data<float, count2, aligned2, OrderRGBA> &v) noexcept
			{
				static_assert(count2 >= 4, "Need extra parameters");
				memcpy(m_data, v.m_data, sizeof(*this));
			}

			ATTR_INLINE float getX() const noexcept;
			ATTR_INLINE float getY() const noexcept;
			ATTR_INLINE float getZ() const noexcept;
			ATTR_INLINE float getW() const noexcept;
			ATTR_INLINE const vec4a getXV() const noexcept;
			ATTR_INLINE const vec4a getYV() const noexcept;
			ATTR_INLINE const vec4a getZV() const noexcept;
			ATTR_INLINE const vec4a getWV() const noexcept;
			ATTR_INLINE const vec4a changeX(float v) const noexcept;
			ATTR_INLINE const vec4a changeY(float v) const noexcept;
			ATTR_INLINE const vec4a changeZ(float v) const noexcept;
			ATTR_INLINE const vec4a changeW(float v) const noexcept;
		};
		template <>
		class vector_constructor<int, 0, true, OrderRGBA, int, int, int, int> 
			:public vector_base<int, 4, true, OrderRGBA>
		{
		public:
			using constructor_t = vector_constructor;
			using vector_base<int, 4, true, OrderRGBA>::m_data;
			using vector_base<int, 4, true, OrderRGBA>::vector_base;
			vector_constructor() = default;
			vector_constructor(const vector_constructor&) = default;
			ATTR_INLINE explicit vector_constructor(int _all) noexcept;
			ATTR_INLINE vector_constructor(const __ivector4 & v) noexcept;
			ATTR_INLINE vector_constructor(int _x, int _y, int _z, int _w) noexcept;
			ATTR_INLINE const vec4a fbits() const noexcept;
			template <typename T2, size_t count2, bool aligned2, typename order2>
			ATTR_INLINE explicit vector_constructor(const vector_data<T2, count2, aligned2, order2> &v) noexcept
			{
				static_assert(count2 >= 4, "Need extra parameters");
				meta::order_util<OrderRGBA, order2, 4>::copy(m_data, v.m_data);
			}
			template <size_t count2, bool aligned2>
			ATTR_INLINE vector_constructor(const vector_data<int, count2, aligned2, OrderRGBA> &v) noexcept
			{
				static_assert(count2 >= 4, "Need extra parameters");
				memcpy(m_data, v.m_data, sizeof(*this));
			}

			ATTR_INLINE int getX() const noexcept;
			ATTR_INLINE int getY() const noexcept;
			ATTR_INLINE int getZ() const noexcept;
			ATTR_INLINE int getW() const noexcept;
			ATTR_INLINE const ivec4a getXV() const noexcept;
			ATTR_INLINE const ivec4a getYV() const noexcept;
			ATTR_INLINE const ivec4a getZV() const noexcept;
			ATTR_INLINE const ivec4a getWV() const noexcept;
			ATTR_INLINE const ivec4a changeX(int v) const noexcept;
			ATTR_INLINE const ivec4a changeY(int v) const noexcept;
			ATTR_INLINE const ivec4a changeZ(int v) const noexcept;
			ATTR_INLINE const ivec4a changeW(int v) const noexcept;
		};
	}
}
