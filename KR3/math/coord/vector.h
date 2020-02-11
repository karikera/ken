#pragma once

#include "vector_method.h"
#include <iostream>

namespace kr
{
	namespace math
	{
		template <typename T, size_t _size, bool aligned, typename order>
		class vector : public Printable<
				vector<T, _size, aligned, order>, 
				AutoComponent, 
				_pri_::vector_method<T, _size, _size, aligned, order>
			>
		{
			using Super = Printable<
				vector,
				AutoComponent,
				_pri_::vector_method<T, _size, _size, aligned, order>
			>;
		public:
			using Super::Super;
			using method_t = _pri_::vector_method<T, _size, _size, aligned, order>;
			using method_t::m_data;
			using method_t::m_vector;

			static const vector makez() noexcept;
			static const vector makes(T v) noexcept;
			static const vector make(const T * ptr) noexcept;

			template <class _Derived, class _Info>
			void writeTo(OutStream<_Derived, _Info>* str) const throws(NotEnoughSpaceException);

			void get(T * ptr) const noexcept;
			const vector movs(const vector & o) const noexcept;
			const vector adds(const vector & o) const noexcept;
			bool operator ==(const vector & o) const noexcept;
			bool operator !=(const vector & o) const noexcept;
			T& operator [](size_t idx) noexcept;
			const T& operator [](size_t idx) const noexcept;
			const vector operator -() const noexcept;
			vector& operator +=(const vector & v) noexcept;
			vector& operator -=(const vector & v) noexcept;
			vector& operator *=(const vector & v) noexcept;
			vector& operator /=(const vector & v) noexcept;
			vector& operator *=(const matrix<T, _size, _size, aligned> & m) noexcept;
			vector& operator *=(T v) noexcept;
			vector& operator /=(T v) noexcept;
			vector& operator &=(const vector & v) noexcept;
			vector& operator |=(const vector & v) noexcept;
			vector& operator ^=(const vector & v) noexcept;
			const vector operator +(const vector & v) const noexcept;
			const vector operator -(const vector & v) const noexcept;
			const vector operator *(const vector & v) const noexcept;
			const vector operator /(const vector & v) const noexcept;
			template <size_t cols>
			const vector<T, cols, aligned, order> operator *(const matrix<T, _size, cols, aligned> & m) const noexcept;
			const vector operator *(T value) const noexcept;
			const vector operator /(T value) const noexcept;
			const vector operator &(const vector & v) const noexcept;
			const vector operator |(const vector & v) const noexcept;
			const vector operator ^(const vector & v) const noexcept;
			T length_sq() const noexcept;
			T length() const noexcept;
			const vector sqrtV() const noexcept;
			const vector length_sqV() const noexcept;
			const vector lengthV() const noexcept;
			const vector normalize() const noexcept;
			bool similar(const vector & v) const noexcept;
			template <uint ... idx> const vector shuffle() const noexcept;

		};;

		template <> const vec4a vec4a::makez() noexcept;
		template <> const vec4a vec4a::makes(float v) noexcept;
		template <> const vec4a vec4a::make(const float * ptr) noexcept;
		template <> void vec4a::get(float * ptr) const noexcept;
		template <> const vec4a vec4a::movs(const vec4a & o) const noexcept;
		template <> const vec4a vec4a::adds(const vec4a & o) const noexcept;
		template <> const vec4a vec4a::operator +(const vec4a & v) const noexcept;
		template <> const vec4a vec4a::operator -(const vec4a & v) const noexcept;
		template <> const vec4a vec4a::operator *(const vec4a & v) const noexcept;
		template <> const vec4a vec4a::operator /(const vec4a & v) const noexcept;
		template <>
		template <>
		const vec4a vec4a::operator *(const mat4a & m) const noexcept;
		template <> const vec4a vec4a::operator *(float value) const noexcept;
		template <> const vec4a vec4a::operator /(float value) const noexcept;
		template <> const vec4a vec4a::operator &(const vec4a & v) const noexcept;
		template <> const vec4a vec4a::operator |(const vec4a & v) const noexcept;
		template <> const vec4a vec4a::operator ^(const vec4a & v) const noexcept;
		template <> float vec4a::length_sq() const noexcept;
		template <> float vec4a::length() const noexcept;
		template <> const vec4a vec4a::length_sqV() const noexcept;
		template <> const vec4a vec4a::lengthV() const noexcept;
		template <> const vec4a vec4a::normalize() const noexcept;

		template <> const ivec4a ivec4a::makez() noexcept;
		template <> const ivec4a ivec4a::makes(int v) noexcept;
		template <> const ivec4a ivec4a::make(const int * ptr) noexcept;
		template <> void ivec4a::get(int * ptr) const noexcept;
		template <> const ivec4a ivec4a::movs(const ivec4a & o) const noexcept;
		template <> const ivec4a ivec4a::adds(const ivec4a & o) const noexcept;
		template <> const ivec4a ivec4a::operator +(const ivec4a & v) const noexcept;
		template <> const ivec4a ivec4a::operator -(const ivec4a & v) const noexcept;
		template <> const ivec4a ivec4a::operator *(const ivec4a & v) const noexcept;
		template <> const ivec4a ivec4a::operator /(const ivec4a & v) const noexcept;
		template <>
		template <>
		const ivec4a ivec4a::operator *(const imat4a & m) const noexcept;
		template <> const ivec4a ivec4a::operator *(int value) const noexcept;
		template <> const ivec4a ivec4a::operator /(int value) const noexcept;
		template <> const ivec4a ivec4a::operator &(const ivec4a & v) const noexcept;
		template <> const ivec4a ivec4a::operator |(const ivec4a & v) const noexcept;
		template <> const ivec4a ivec4a::operator ^(const ivec4a & v) const noexcept;
		template <> int ivec4a::length_sq() const noexcept;
		template <> int ivec4a::length() const noexcept;
		template <> const ivec4a ivec4a::length_sqV() const noexcept;
		template <> const ivec4a ivec4a::lengthV() const noexcept;
		template <> const ivec4a ivec4a::normalize() const noexcept;

		namespace _pri_
		{
			template <size_t counter, size_t maximum, size_t ... v>
			struct makeshuffle;

			template <size_t counter, size_t maximum, size_t first, size_t ... v>
			struct makeshuffle<counter, maximum, first, v ...>
			{
				static constexpr uint value = (makeshuffle<counter - 1, maximum + 1, v ...>::value << 2) | first;
			};
			template <size_t maximum, size_t first, size_t ... v>
			struct makeshuffle<0, maximum, first, v ...>
			{
				static constexpr uint value = 0;
			};


			template <size_t counter, size_t maximum>
			struct makeshuffle<counter, maximum>
			{
				static constexpr uint value = (makeshuffle<counter - 1, maximum + 1>::value << 2) | maximum;
			};
			template <size_t maximum>
			struct makeshuffle<0, maximum>
			{
				static constexpr uint value = 0;
			};
		}

		template <typename T, size_t _size, bool aligned, typename order>
		template <uint ... idx>
		const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::shuffle() const noexcept
		{
			static_assert(sizeof ...(idx) <= _size, "index overflow");
			vector<T, _size, aligned, order> out;
			size_t i = 0;
			unpack(out.m_data[i++] = m_data[idx]);
			for (; i != _size; i++)
			{
				out.m_data[i] = m_data[i];
			}
			return out;
		}

		template <>
		template <uint ... idx>
		const vec4a vec4a::shuffle() const noexcept
		{
			static_assert(sizeof ...(idx) <= 4, "index overflow");
#if defined(EMSIMD) || defined(MSSIMD)
			constexpr uint shuffleKey = _pri_::makeshuffle<4, 0, idx ...>::value;
			return _mm_shuffle_ps(m_vector, m_vector, shuffleKey);
#else
			vec4a out;
			size_t i = 0;
			unpack(out.m_vector[i++] = m_vector[idx]);
			for (; i != 4; i++)
			{
				out.m_vector[i] = m_vector[i];
			}
			return out;
#endif
		}

		template <>
		template <uint ... idx>
		const ivec4a ivec4a::shuffle() const noexcept
		{
			static_assert(sizeof ...(idx) <= 4, "index overflow");
#if defined(EMSIMD) || defined(MSSIMD)
			constexpr uint shuffleKey = _pri_::makeshuffle<4, 0, idx ...>::value;
			return _mm_shuffle_epi32(m_vector, shuffleKey);
#else
			ivec4a out;
			size_t i = 0;
			unpack(out.m_vector[i++] = m_vector[idx]);
			for (; i != 4; i++)
			{
				out.m_vector[i] = m_vector[i];
			}
			return out;
#endif
		}
	}
}

template <typename T, size_t _size, bool aligned, typename order> 
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> lerp(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b, T rate) noexcept;
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> lerp(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b, const kr::math::vector<T, _size, aligned, order> & rate) noexcept;
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> lerpf(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b, float rate) noexcept;
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE T dot(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b) noexcept;
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE T dot3(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b) noexcept;
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> dotV(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b) noexcept;
template <typename T, bool aligned, typename order>
ATTR_INLINE const T cross(const kr::math::vector<T, 2, aligned, order> & a, const kr::math::vector<T, 2, aligned, order> & b) noexcept;
template <typename T, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, 4, aligned, order> cross(const kr::math::vector<T, 4, aligned, order> & a, const kr::math::vector<T, 4, aligned, order> & b) noexcept;
template <typename T, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, 3, aligned, order> cross(const kr::math::vector<T, 3, aligned, order> & a, const kr::math::vector<T, 3, aligned, order> & b) noexcept;
template <> 
ATTR_INLINE const kr::vec4a dotV<float, 4, true>(const kr::vec4a & a, const kr::vec4a & b) noexcept;
template <>
ATTR_INLINE const kr::ivec4a dotV<int, 4, true>(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;

template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> operator *(T value, const kr::math::vector<T, _size, aligned, order> & b) noexcept;
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> operator /(T value, const kr::math::vector<T, _size, aligned, order> & b) noexcept;

inline const kr::vec4a roundv(const kr::vec4a& V) noexcept;
inline const kr::vec4a cosv(const kr::vec4a& V) noexcept;
inline const kr::vec4a sinv(const kr::vec4a& V) noexcept;
inline const kr::vec4a atanv(const kr::vec4a& V) noexcept;
inline const kr::vec4a atan2v(const kr::vec4a& Y, const kr::vec4a& X) noexcept;
inline const kr::vec4a angmodv(const kr::vec4a & angles) noexcept;
inline const kr::vec4a slerpv(const kr::vec4a& a, const kr::vec4a& b, const kr::vec4a & rate) noexcept;
inline void sincosv(kr::vec4a * s, kr::vec4a * c, const kr::vec4a& rad) noexcept;

template <size_t ... idx>
ATTR_INLINE const kr::vec4a shuffle(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
	static_assert(sizeof ...(idx) <= 4, "index overflow");
#if defined(EMSIMD) || defined(MSSIMD)
	constexpr kr::uint shuffleKey = kr::math::_pri_::makeshuffle<4, 0, idx ...>::value;
	return _mm_shuffle_ps(a.m_vector, b.m_vector, shuffleKey);
#else
	size_t v[] ={idx...};
	return { a[v[0]], a[v[1]], b[v[2]], b[v[3]] };
#endif
}
ATTR_INLINE const kr::vec4a unpack_low(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a unpack_high(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a sqrtv(const kr::vec4a & v) noexcept;
ATTR_INLINE const kr::vec4a maxv(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a minv(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a clampv(const kr::vec4a & a, const kr::vec4a & v, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4 clampv(const kr::vec4& a, const kr::vec4& v, const kr::vec4& b) noexcept;
ATTR_INLINE const kr::vec4 clamp01v(const kr::vec4& a, const kr::vec4& v, const kr::vec4& b) noexcept;
ATTR_INLINE const kr::vec4a absv(const kr::vec4a& V) noexcept;
ATTR_INLINE const kr::vec4a mad(const kr::vec4a & a, const kr::vec4a & b, const kr::vec4a & c) noexcept;
ATTR_INLINE const kr::vec4a andnot(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a operator <=(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a operator >=(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a operator <(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a operator >(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a equal(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a notequal(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::vec4a vnot(const kr::vec4a & a, const kr::vec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a mad(const kr::ivec4a & a, const kr::ivec4a & b, const kr::ivec4a & c) noexcept;
ATTR_INLINE const kr::ivec4a andnot(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a operator <=(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a operator >=(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a operator <(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a operator >(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a equal(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a notequal(const kr::ivec4a & a, const kr::ivec4a & b) noexcept;
ATTR_INLINE const kr::ivec4a operator !(const kr::ivec4a & a) noexcept;
