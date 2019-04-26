#pragma once

#include "vector.h"
#include "matrix.h"

#ifdef WIN32
#include "mmx.h"
#endif

static const kr::vec4a g_XMHalfPi = { kr::math::pi / 2, kr::math::pi / 2, kr::math::pi / 2, kr::math::pi / 2 };
static const kr::vec4a g_XMTwoPi = { kr::math::pi * 2, kr::math::pi * 2, kr::math::pi * 2, kr::math::pi * 2 };
static const kr::vec4a g_XMPi = { kr::math::pi, kr::math::pi, kr::math::pi, kr::math::pi };

static const kr::vec4a g_XMZero = { 0.0f, 0.0f, 0.0f, 0.0f };
static const kr::vec4a g_XMOne = { 1.0f, 1.0f, 1.0f, 1.0f };
static const kr::vec4a g_XMNegativeOne = { -1.0f,-1.0f,-1.0f,-1.0f };
static const kr::vec4a g_XMNegativeZero = kr::ivec4a((int)0x80000000).fbits();

static const kr::vec4a g_XMSinCoefficients1 = { -2.3889859e-08f, -0.16665852f /*Est1*/, +0.0083139502f /*Est2*/, -0.00018524670f /*Est3*/ };
static const kr::vec4a g_XMCosCoefficients1 = { -2.6051615e-07f, -0.49992746f /*Est1*/, +0.041493919f /*Est2*/, -0.0012712436f /*Est3*/ };
static const kr::vec4a g_XMATanEstCoefficients0 = { +0.999866f, +0.999866f, +0.999866f, +0.999866f };
static const kr::vec4a g_XMATanEstCoefficients1 = { -0.3302995f, +0.180141f, -0.085133f, +0.0208351f };

static const kr::vec4a g_XMReciprocalTwoPi = { 0.5f / kr::math::pi, 0.5f / kr::math::pi, 0.5f / kr::math::pi, 0.5f / kr::math::pi };
static const kr::vec4a g_XMNoFraction = { 8388608.0f,8388608.0f,8388608.0f,8388608.0f };
static const kr::vec4a g_XMAbsMask = kr::ivec4a((int)0x7FFFFFFF).fbits();
static const kr::vec4a g_XMInfinity = kr::ivec4a(0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000).fbits();
static const kr::vec4a g_XMIdentityR0 = { 1.0f, 0.0f, 0.0f, 0.0f };

namespace kr
{
	namespace math
	{
		template<typename T, size_t _size, bool aligned, typename order>
		template <class _Derived, class _Info>
		void vector<T, _size, aligned, order>::writeTo(OutStream<_Derived, _Info> *str) const // NotEnoughSpaceException
		{
			*str << '{' << m_data[0];
			for (size_t i = 1; i < _size; i++)
			{
				*str << ", " << m_data[i];
			}
			*str << '}';
		}

		template<typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::makez() noexcept
		{
			vector<T, _size, aligned, order> ret;
			for (size_t i = 0; i < _size; i++)
			{
				ret.m_data[i] = 0;
			}
			return ret;
		}
		template<typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::makes(T v) noexcept
		{
			vector<T, _size, aligned, order> ret;
			ret.m_data[0] = v;
			for (size_t i = 1; i < _size; i++)
			{
				ret.m_data[i] = 0;
			}
			return ret;
		}
		template<typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::make(const T * ptr) noexcept
		{
			return *(vector<T, _size, aligned, order>*)ptr;
		}
		template<typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE void vector<T, _size, aligned, order>::get(T * ptr) const noexcept
		{
			*(vector*)ptr = *this;
		}
		template<typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::movs(const vector<T, _size, aligned, order> & o) const noexcept
		{
			vector<T, _size, aligned, order> out;
			out[0] = o[0];
			for (size_t i = 1; i < _size; i++)
			{
				out[i] = m_data[i];
			}
			return out;
		}
		template<typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::adds(const vector<T, _size, aligned, order> & o) const noexcept
		{
			vector<T, _size, aligned, order> out;
			out[0] = m_data[0] + o[0];
			for (size_t i = 1; i < _size; i++)
			{
				out[i] = m_data[i];
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE bool vector<T, _size, aligned, order>::operator ==(const vector<T, _size, aligned, order> & o) const  noexcept
		{
			return memc<_size * sizeof(T)>::equals(m_data, o.m_data);
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE bool vector<T, _size, aligned, order>::operator !=(const vector<T, _size, aligned, order> & o) const  noexcept
		{
			return !memc<_size * sizeof(T)>::equals(m_data, o.m_data);
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE T& vector<T, _size, aligned, order>::operator [](size_t idx)  noexcept
		{
			return m_data[idx];
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const T& vector<T, _size, aligned, order>::operator [](size_t idx) const noexcept
		{
			return m_data[idx];
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator -() const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = -m_data[i];
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator +=(const vector<T, _size, aligned, order> & v) noexcept
		{
			return *this = *this + v;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator -=(const vector<T, _size, aligned, order> & v) noexcept
		{
			return *this = *this - v;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator *=(const vector<T, _size, aligned, order> & v) noexcept
		{
			return *this = *this * v;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator /=(const vector<T, _size, aligned, order> & v) noexcept
		{
			return *this = *this / v;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator *=(const matrix<T, _size, _size, aligned> & m) noexcept
		{
			return *this = *this * m;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator *=(T v) noexcept
		{
			return *this = *this * v;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator /=(T v) noexcept
		{
			return *this = *this / v;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator &=(const vector<T, _size, aligned, order> & m) noexcept
		{
			return *this = *this & m;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator |=(const vector<T, _size, aligned, order> & m) noexcept
		{
			return *this = *this | m;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE vector<T, _size, aligned, order>& vector<T, _size, aligned, order>::operator ^=(const vector<T, _size, aligned, order> & m) noexcept
		{
			return *this = *this ^ m;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator +(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = m_data[i] + v.m_data[i];
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator -(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = m_data[i] - v.m_data[i];
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator *(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = m_data[i] * v.m_data[i];
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator /(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = m_data[i] / v.m_data[i];
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		template <size_t cols>
		ATTR_INLINE const vector<T, cols, aligned, order> vector<T, _size, aligned, order>::operator *(const matrix<T, _size, cols, aligned> & m) const noexcept
		{
			constexpr size_t _cols = _size;
			constexpr size_t rows = _size;

			vector<T, _cols, aligned, order> out;
			for (size_t j = 0; j < _cols; j++)
			{
				T sum = (*this)[0] * m.v[0][j];
				for (size_t k = 1; k < rows; k++)
				{
					sum += (*this)[k] * m.v[k][j];
				}
				out.m_data[j] = sum;
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator *(T value) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = m_data[i] * value;
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator /(T value) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = m_data[i] / value;
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator &(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				raw(out.m_data[i]) = raw(m_data[i]) & raw(v.m_data[i]);
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator |(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				raw(out.m_data[i]) = raw(m_data[i]) | raw(v.m_data[i]);
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::operator ^(const vector<T, _size, aligned, order> & v) const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				raw(out.m_data[i]) = raw(m_data[i]) ^ raw(v.m_data[i]);
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE T vector<T, _size, aligned, order>::length_sq() const noexcept
		{
			return dot(*this, *this);
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE T vector<T, _size, aligned, order>::length() const noexcept
		{
			return (T)sqrt((T)length_sq());
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::sqrtV() const noexcept
		{
			vector<T, _size, aligned, order> out;
			for (size_t i = 0; i < _size; i++)
			{
				out.m_data[i] = (T)sqrt((T)m_data[i]);
			}
			return out;
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::length_sqV() const noexcept
		{
			return vector<T, _size, aligned, order>(length_sq());
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::lengthV() const noexcept
		{
			return vector<T, _size, aligned, order>(length());
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE const vector<T, _size, aligned, order> vector<T, _size, aligned, order>::normalize() const noexcept
		{
			return *this / length();
		}
		template <typename T, size_t _size, bool aligned, typename order>
		ATTR_INLINE bool vector<T, _size, aligned, order>::similar(const vector<T, _size, aligned, order> & v) const noexcept
		{
			const float almostZero = 1.0e-6f;
			return (*this - v).length_sq() <= almostZero;
		}

	}
}

inline kr::vec4a XMVectorSelect(const kr::vec4a & V1, const kr::vec4a & V2, const kr::vec4a & Control) noexcept
{
	kr::vec4a vTemp1 = andnot(Control, V1);
	kr::vec4a vTemp2 = V2 & Control;
	return vTemp1 | vTemp2;
}
inline kr::vec4a XMVectorTrueInt() noexcept
{
	return kr::ivec4a(-1).fbits();
}
inline kr::vec4a XMVectorEqualInt(const kr::vec4a& V1, const kr::vec4a& V2) noexcept
{
	return equal(V1.ibits(), V2.ibits()).fbits();
}
inline kr::vec4a XMVectorOrInt(const kr::vec4a& V1, const kr::vec4a& V2) noexcept
{
	return (V1.ibits() | V2.ibits()).fbits();
}
inline kr::vec4a XMVectorAndInt(const kr::vec4a& V1, const kr::vec4a& V2) noexcept
{
	return V1 & V2;
}
inline kr::vec4a XMVectorIsInfinite(const kr::vec4a& V) noexcept
{
	// Mask off the sign bit
	kr::vec4a vTemp = V & g_XMAbsMask;
	// Compare to infinity
	vTemp = equal(vTemp, g_XMInfinity);
	// If any are infinity, the signs are true.
	return vTemp;
}
inline kr::vec4a XMVectorReciprocalEst(const kr::vec4a& V) noexcept
{
#if defined(EMSIMD) || defined(MSSIMD)
	return _mm_rcp_ps(V.m_vector);
#else
	return kr::vec4a{
		1.f / V.x,
		1.f / V.y,
		1.f / V.z,
		1.f / V.w,
	};
#endif
}

inline const kr::vec4a roundv(const kr::vec4a& V) noexcept
{
	kr::vec4a sign = V & g_XMNegativeZero;
	kr::vec4a sMagic = g_XMNoFraction | sign;
	kr::vec4a R1 = V + sMagic;
	R1 = R1 - sMagic;
	kr::vec4a R2 = V & g_XMAbsMask;
	kr::vec4a mask = R2 <= g_XMNoFraction;
	R2 = andnot(mask, V);
	R1 = R1 & mask;
	kr::vec4a vResult = R1 ^ R2;
	return vResult;
}
inline const kr::vec4a cosv(const kr::vec4a& V) noexcept
{
	// Map V to x in [-pi,pi].
	kr::vec4a x = angmodv(V);

	// Map in [-pi/2,pi/2] with cos(y) = sign*cos(x).
	kr::vec4a sign = x & g_XMNegativeZero;
	kr::vec4a c = g_XMPi | sign;  // pi when x >= 0, -pi when x < 0
	kr::vec4a absx = andnot(sign, x);  // |x|
	kr::vec4a rflx = c - x;
	kr::vec4a comp = absx <= g_XMHalfPi;
	kr::vec4a select0 = comp & x;
	kr::vec4a select1 = andnot(comp, rflx);
	x = select0 | select1;
	select0 = comp & g_XMOne;
	select1 = andnot(comp, g_XMNegativeOne);
	sign = select0 | select1;

	kr::vec4a x2 = x * x;

	// Compute polynomial approximation
	const kr::vec4a CEC = g_XMCosCoefficients1;
	kr::vec4a vConstants = CEC.getWV();
	kr::vec4a Result = vConstants * x2;

	vConstants = CEC.getZV();
	Result = Result + vConstants;
	Result = Result * x2;

	vConstants = CEC.getYV();
	Result = Result + vConstants;
	Result = Result * x2;

	Result = Result + g_XMOne;
	Result = Result * sign;
	return Result;
}
inline const kr::vec4a sinv(const kr::vec4a& V) noexcept
{
	// 7-degree minimax approximation

	// Force the value within the bounds of pi
	kr::vec4a x = angmodv(V);

	// Map in [-pi/2,pi/2] with sin(y) = sin(x).
	kr::vec4a sign = x & g_XMNegativeZero;
	kr::vec4a c = g_XMPi | sign;  // pi when x >= 0, -pi when x < 0
	kr::vec4a absx = andnot(sign, x);  // |x|
	kr::vec4a rflx = c - x;
	kr::vec4a comp = absx <= g_XMHalfPi;
	kr::vec4a select0 = comp & x;
	kr::vec4a select1 = andnot(comp, rflx);
	x = select0 | select1;

	kr::vec4a x2 = x * x;

	// Compute polynomial approximation
	const kr::vec4a SEC = g_XMSinCoefficients1;
	kr::vec4a vConstants = SEC.getWV();
	kr::vec4a Result = vConstants * x2;

	vConstants = SEC.getZV();
	Result = Result + vConstants;
	Result = Result * x2;

	vConstants = SEC.getYV();
	Result = Result + vConstants;
	Result = Result * x2;

	Result = Result + g_XMOne;
	Result = Result * x;
	return Result;
}
inline const kr::vec4a atanv(const kr::vec4a& V) noexcept
{
	// 9-degree minimax approximation

	kr::vec4a absV = absv(V);
	kr::vec4a invV = g_XMOne / V;
	kr::vec4a comp = V > g_XMOne;
	kr::vec4a select0 = comp & g_XMOne;
	kr::vec4a select1 = andnot(comp, g_XMNegativeOne);
	kr::vec4a sign = select0 | select1;
	comp = absV <= g_XMOne;
	select0 = comp & g_XMZero;
	select1 = andnot(comp, sign);
	sign = select0 | select1;
	select0 = comp & V;
	select1 = andnot(comp, invV);
	kr::vec4a x = select0 | select1;

	kr::vec4a x2 = x * x;

	// Compute polynomial approximation
	const kr::vec4a AEC = g_XMATanEstCoefficients1;
	kr::vec4a vConstants = AEC.getWV();
	kr::vec4a Result = vConstants * x2;

	vConstants = AEC.getZV();
	Result = Result + vConstants;
	Result = Result * x2;

	vConstants = AEC.getYV();
	Result = Result + vConstants;
	Result = Result * x2;

	vConstants = AEC.getXV();
	Result = Result + vConstants;
	Result = Result * x2;

	// ATanEstCoefficients0 is already splatted
	Result = Result + g_XMATanEstCoefficients0;
	Result = Result * x;
	kr::vec4a result1 = sign * g_XMHalfPi;
	result1 = result1 - Result;

	comp = equal(sign, g_XMZero);
	select0 = comp & Result;
	select1 = andnot(comp, result1);
	Result = select0 | select1;
	return Result;
}
inline const kr::vec4a atan2v(const kr::vec4a& Y, const kr::vec4a& X) noexcept
{
	static const kr::vec4a ATan2Constants = { kr::math::pi, kr::math::pi / 2, kr::math::pi / 4, kr::math::pi * 3.f / 4.f };

	const kr::vec4a Zero = kr::vec4a::makez();
	kr::vec4a ATanResultValid = XMVectorTrueInt();

	kr::vec4a Pi = ATan2Constants.getXV();
	kr::vec4a PiOverTwo = ATan2Constants.getYV();
	kr::vec4a PiOverFour = ATan2Constants.getZV();
	kr::vec4a ThreePiOverFour = ATan2Constants.getWV();

	kr::vec4a YEqualsZero = equal(Y, Zero);
	kr::vec4a XEqualsZero = equal(X, Zero);
	kr::vec4a XIsPositive = XMVectorAndInt(X, g_XMNegativeZero);
	XIsPositive = XMVectorEqualInt(XIsPositive, Zero);
	kr::vec4a YEqualsInfinity = XMVectorIsInfinite(Y);
	kr::vec4a XEqualsInfinity = XMVectorIsInfinite(X);

	kr::vec4a YSign = XMVectorAndInt(Y, g_XMNegativeZero);
	Pi = XMVectorOrInt(Pi, YSign);
	PiOverTwo = XMVectorOrInt(PiOverTwo, YSign);
	PiOverFour = XMVectorOrInt(PiOverFour, YSign);
	ThreePiOverFour = XMVectorOrInt(ThreePiOverFour, YSign);

	kr::vec4a R1 = XMVectorSelect(Pi, YSign, XIsPositive);
	kr::vec4a R2 = XMVectorSelect(ATanResultValid, PiOverTwo, XEqualsZero);
	kr::vec4a R3 = XMVectorSelect(R2, R1, YEqualsZero);
	kr::vec4a R4 = XMVectorSelect(ThreePiOverFour, PiOverFour, XIsPositive);
	kr::vec4a R5 = XMVectorSelect(PiOverTwo, R4, XEqualsInfinity);
	kr::vec4a Result = XMVectorSelect(R3, R5, YEqualsInfinity);
	ATanResultValid = XMVectorEqualInt(Result, ATanResultValid);

	kr::vec4a Reciprocal = XMVectorReciprocalEst(X);
	kr::vec4a V = Y * Reciprocal;
	kr::vec4a R0 = atanv(V);

	R1 = XMVectorSelect(Pi, g_XMNegativeZero, XIsPositive);
	R2 = R0 + R1;

	Result = XMVectorSelect(Result, R2, ATanResultValid);

	return Result;
}
inline const kr::vec4a angmodv(const kr::vec4a & angles) noexcept
{
	using kr::vec4a;

	// Modulo the range of the given angles such that -XM_PI <= Angles < XM_PI
	vec4a vResult = angles * g_XMReciprocalTwoPi;
	// Use the inline function due to complexity for rounding
	vResult = roundv(vResult);
	vResult = vResult * g_XMTwoPi;
	vResult = angles - vResult;
	return vResult;
}
inline const kr::vec4a slerpv(const kr::vec4a& a, const kr::vec4a& b, const kr::vec4a & rate) noexcept
{
	_assert(rate.y == rate.x && rate.z == rate.x && rate.w == rate.x);

	static const kr::vec4a OneMinusEpsilon = { 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f };
	static const kr::vec4a SignMask2 = { 0x80000000,0x00000000,0x00000000,0x00000000 };
	static const kr::vec4a MaskXY = { 0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000 };

	kr::vec4a CosOmega = dotV((kr::vec4a&)a, (kr::vec4a&)b);

	const kr::vec4a Zero = kr::vec4a::makez();
	kr::vec4a Control = CosOmega < Zero;
	kr::vec4a Sign = XMVectorSelect(g_XMOne, g_XMNegativeOne, Control);

	CosOmega = CosOmega * Sign;

	Control = CosOmega < OneMinusEpsilon;

	kr::vec4a SinOmega = CosOmega * CosOmega;
	SinOmega = g_XMOne < SinOmega;
	SinOmega = sqrtv(SinOmega);

	kr::vec4a Omega = atan2v(SinOmega, CosOmega);

	kr::vec4a V01 = rate.shuffle<1, 0, 3, 2>();
	V01 = V01 & MaskXY;
	V01 = V01 ^ SignMask2;
	V01 = g_XMIdentityR0 + V01;

	kr::vec4a S0 = V01 * Omega;
	S0 = sinv(S0);
	S0 = S0 / SinOmega;

	S0 = XMVectorSelect(V01, S0, Control);

	kr::vec4a S1 = S0.getYV();
	S0 = S0.getXV();

	S1 = S1 * Sign;
	kr::vec4a Result = (kr::vec4a&)a * S0;
	S1 = S1 * (kr::vec4a&)b;
	Result = Result + S1;
	return Result;
}
inline void sincosv(kr::vec4a * psin, kr::vec4a * pcos, const kr::vec4a& rad) noexcept
{
	using kr::vec4a;

	// Force the value within the bounds of pi
	vec4a x = angmodv(rad);

	// Map in [-pi/2,pi/2] with sin(y) = sin(x), cos(y) = sign*cos(x).
	vec4a sign = x & g_XMNegativeZero;
	vec4a c = g_XMPi | sign;  // pi when x >= 0, -pi when x < 0
	vec4a absx = andnot(sign, x);  // |x|
	vec4a rflx = c - x;
	vec4a comp = absx <= g_XMHalfPi;
	vec4a select0 = comp & x;
	vec4a select1 = andnot(comp, rflx);
	x = select0 | select1;
	select0 = comp & kr::math::CV_1;
	select1 = andnot(comp, g_XMNegativeOne);
	sign = select0 | select1;

	vec4a x2 = x * x;

	// Compute polynomial approximation for sine
	const vec4a SEC = g_XMSinCoefficients1;
	vec4a vConstants = SEC.getWV();
	vec4a Result = vConstants * x2;

	vConstants = SEC.getZV();
	Result = Result + vConstants;
	Result = Result * x2;

	vConstants = SEC.getYV();
	Result = Result + vConstants;
	Result = Result * x2;

	Result = Result + kr::math::CV_1;
	Result = Result * x;
	*psin = Result;

	// Compute polynomial approximation for cosine
	const vec4a CEC = g_XMCosCoefficients1;
	vConstants = CEC.getWV();
	Result = vConstants * x2;

	vConstants = CEC.getZV();
	Result = Result + vConstants;
	Result = Result * x2;

	vConstants = CEC.getXV();
	Result = Result + vConstants;
	Result = Result * x2;

	Result = Result + kr::math::CV_1;
	Result = Result * sign;
	*pcos = Result;
}

ATTR_INLINE const kr::vec4a unpack_low(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_unpacklo_ps(a.m_vector, b.m_vector);
#else
	return kr::vec4a(a.x, b.x, a.y, b.y);
#endif
}
ATTR_INLINE const kr::vec4a unpack_high(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_unpackhi_ps(a.m_vector, b.m_vector);
#else
	return kr::vec4a(a.z, b.z, a.w, b.w);
#endif
}
ATTR_INLINE const kr::vec4a sqrtv(const kr::vec4a & v) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_sqrt_ps(v.m_vector);
#else
	return kr::vec4a(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w));
#endif
}
ATTR_INLINE const kr::vec4a maxv(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_max_ps(a.m_vector, b.m_vector);
#else
	return kr::vec4a(
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y,
		a.z > b.z ? a.z : b.z,
		a.w > b.w ? a.w : b.w
	);
#endif
}
ATTR_INLINE const kr::vec4a minv(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_min_ps(a.m_vector, b.m_vector);
#else
	return kr::vec4a(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y,
		a.z < b.z ? a.z : b.z,
		a.w < b.w ? a.w : b.w
	);
#endif
}
ATTR_INLINE const kr::vec4a clampv(const kr::vec4a & a, const kr::vec4a & v, const kr::vec4a & b) noexcept
{
#ifdef USE_SIMD
	return minv(maxv(v, a), b);
#else
	return kr::vec4a(
		kr::math::clamp(a.x, v.x, b.x),
		kr::math::clamp(a.y, v.y, b.y),
		kr::math::clamp(a.z, v.z, b.z),
		kr::math::clamp(a.w, v.w, b.w)
	);
#endif
}
ATTR_INLINE const kr::vec4a absv(const kr::vec4a& V) noexcept
{
	return maxv(kr::vec4a::makez() - V, V);
}
ATTR_INLINE const kr::vec4a mad(const kr::vec4a & a, const kr::vec4a & b, const kr::vec4a & c) noexcept
{
#ifdef _AVX2_
	return _mm_fmadd_ps(a.m_vector, b.m_vector, c.m_vector);
#else
	return a * b + c;
#endif
}
ATTR_INLINE const kr::vec4a andnot(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_andnot_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		~(int&)a.x & (int&)b.x,
		~(int&)a.y & (int&)b.y,
		~(int&)a.z & (int&)b.z,
		~(int&)a.w & (int&)b.w
	).fbits();
#endif
}
ATTR_INLINE const kr::vec4a operator <=(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_cmple_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x <= b.x ? -1 : 0,
		a.y <= b.y ? -1 : 0,
		a.z <= b.z ? -1 : 0,
		a.w <= b.w ? -1 : 0
	).fbits();
#endif
}
ATTR_INLINE const kr::vec4a operator >=(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_cmpge_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x >= b.x ? -1 : 0,
		a.y >= b.y ? -1 : 0,
		a.z >= b.z ? -1 : 0,
		a.w >= b.w ? -1 : 0
	).fbits();
#endif
}
ATTR_INLINE const kr::vec4a operator <(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_cmplt_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x < b.x ? -1 : 0,
		a.y < b.y ? -1 : 0,
		a.z < b.z ? -1 : 0,
		a.w < b.w ? -1 : 0
	).fbits();
#endif
}
ATTR_INLINE const kr::vec4a operator >(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_cmpgt_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x > b.x ? -1 : 0,
		a.y > b.y ? -1 : 0,
		a.z > b.z ? -1 : 0,
		a.w > b.w ? -1 : 0
	).fbits();
#endif
}
ATTR_INLINE const kr::vec4a equal(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_cmpeq_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x == b.x ? -1 : 0,
		a.y == b.y ? -1 : 0,
		a.z == b.z ? -1 : 0,
		a.w == b.w ? -1 : 0
	).fbits();
#endif
}
ATTR_INLINE const kr::vec4a notequal(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
#if defined(MSSIMD) || defined(EMSIMD)
	return _mm_cmpneq_ps(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x != b.x ? -1 : 0,
		a.y != b.y ? -1 : 0,
		a.z != b.z ? -1 : 0,
		a.w != b.w ? -1 : 0
	).fbits();
#endif
}
ATTR_INLINE const kr::ivec4a mad(const kr::ivec4a & a, const kr::ivec4a & b, const kr::ivec4a & c) noexcept
{
#ifdef _AVX2_
	return _mm_fmadd_ps(a.m_vector, b.m_vector, c.m_vector);
#else
	return a*b + c;
#endif
}
ATTR_INLINE const kr::ivec4a andnot(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
#ifdef EMSIMD
	return emscripten_int32x4_and(emscripten_int32x4_not(a.m_vector), b.m_vector);
#elif defined(MSSIMD)
	return _mm_andnot_si128(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		~a.x & b.x,
		~a.y & b.y,
		~a.z & b.z,
		~a.w & b.w
	);
#endif
}
ATTR_INLINE const kr::ivec4a operator <=(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
#ifdef EMSIMD
	return emscripten_int32x4_lessThanOrEqual(a.m_vector, b.m_vector);
#elif defined(MSSIMD)
	return equal((kr::ivec4a)_mm_min_epu32(a.m_vector, b.m_vector), a);
#else
	return kr::ivec4a(
		a.x <= b.x ? -1 : 0,
		a.y <= b.y ? -1 : 0,
		a.z <= b.z ? -1 : 0,
		a.w <= b.w ? -1 : 0
	);
#endif
}
ATTR_INLINE const kr::ivec4a operator >=(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
	return b <= a;
}
ATTR_INLINE const kr::ivec4a operator <(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
#ifdef EMSIMD
	return emscripten_int32x4_lessThan(a.m_vector, b.m_vector);
#elif defined(MSSIMD)
	return _mm_cmplt_epi32(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x < b.x ? -1 : 0,
		a.y < b.y ? -1 : 0,
		a.z < b.z ? -1 : 0,
		a.w < b.w ? -1 : 0
	);
#endif
}
ATTR_INLINE const kr::ivec4a operator >(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
#ifdef EMSIMD
	return emscripten_int32x4_greaterThan(a.m_vector, b.m_vector);
#elif defined(MSSIMD)
	return _mm_cmpgt_epi32(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x > b.x ? -1 : 0,
		a.y > b.y ? -1 : 0,
		a.z > b.z ? -1 : 0,
		a.w > b.w ? -1 : 0
	);
#endif
}
ATTR_INLINE const kr::ivec4a equal(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
#ifdef EMSIMD
	return emscripten_int32x4_equal(a.m_vector, b.m_vector);
#elif defined(MSSIMD)
	return _mm_cmpeq_epi32(a.m_vector, b.m_vector);
#else
	return kr::ivec4a(
		a.x == b.x ? -1 : 0,
		a.y == b.y ? -1 : 0,
		a.z == b.z ? -1 : 0,
		a.w == b.w ? -1 : 0
	);
#endif
}
ATTR_INLINE const kr::ivec4a notequal(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
#ifdef USE_SIMD
	return !equal(a, b);
#else
	return kr::ivec4a(
		a.x != b.x ? -1 : 0,
		a.y != b.y ? -1 : 0,
		a.z != b.z ? -1 : 0,
		a.w != b.w ? -1 : 0
	);
#endif
}
ATTR_INLINE const kr::ivec4a operator !(const kr::ivec4a & a) noexcept
{
#ifdef USE_SIMD
	return a ^ kr::math::CV_ALL_BITS;
#else
	return kr::ivec4a(
		~a.x,
		~a.y,
		~a.z,
		~a.w
	);
#endif
}

template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> operator *(T value, const kr::math::vector<T, _size, aligned, order> & b) noexcept
{
	kr::math::vector<T, _size, aligned, order> out;
	for (size_t i = 0; i < _size; i++)
	{
		out.m_data[i] = value * b.m_data[i];
	}
	return out;
}
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> operator /(T value, const kr::math::vector<T, _size, aligned, order> & b) noexcept
{
	kr::math::vector<T, _size, aligned, order> out;
	for (size_t i = 0; i < _size; i++)
	{
		out.m_data[i] = value / b.m_data[i];
	}
	return out;
}

template <typename T, size_t _size, bool aligned, typename order> 
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> lerp(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b, T rate) noexcept
{
	return ((b - a) * rate + a);
}
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> lerp(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b, const kr::math::vector<T, _size, aligned, order> & rate) noexcept
{
	return ((b - a) * rate + a);
}
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> lerpf(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b, float rate) noexcept
{
	return (kr::math::vector<T, _size, aligned, order>)((kr::math::vector<float, _size>)(b - a) * rate) + a;
}

template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE T dot(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b) noexcept
{
	kr::math::vector<T, _size, aligned, order> mult = a * b;
	T sum = mult[0];
	for (size_t i = 1; i < _size; i++)
	{
		sum += mult[i];
	}
	return sum;
}
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE T dot3(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b) noexcept
{
	kr::math::vector<T, _size, aligned, order> mult = a * b;

	constexpr size_t sz = (_size > 3 ? 3 : _size);
	T sum = mult[0];
	for (size_t i = 1; i < sz; i++)
	{
		sum += mult[i];
	}
	return sum;
}
template <typename T, size_t _size, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, _size, aligned, order> dotV(const kr::math::vector<T, _size, aligned, order> & a, const kr::math::vector<T, _size, aligned, order> & b) noexcept
{
	return kr::math::vector<T, _size, aligned, order>(dot(a, b));
}
template <typename T, bool aligned, typename order>
ATTR_INLINE const T cross(const kr::math::vector<T, 2, aligned, order>& a, const kr::math::vector<T, 2, aligned, order>& b) noexcept
{
	return (T)(a.x * b.y - b.x * a.y);
}
template <typename T, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, 3, aligned, order> cross(const kr::math::vector<T, 3, aligned, order>& a, const kr::math::vector<T, 3, aligned, order>& b) noexcept
{
	return{ (T)(a.y*b.z - b.y*a.z), (T)(a.z*b.x - b.z*a.x),  (T)(a.x*b.y - b.x*a.y) };
}
template <typename T, bool aligned, typename order>
ATTR_INLINE const kr::math::vector<T, 4, aligned, order> cross(const kr::math::vector<T, 4, aligned, order>& a, const kr::math::vector<T, 4, aligned, order>& b) noexcept
{
	return{ (T)(a.y*b.z - b.y*a.z), (T)(a.z*b.x - b.z*a.x),  (T)(a.x*b.y - b.x*a.y), (T)(a.w*b.w) };
}

namespace kr
{

	template<>
	ATTR_INLINE const vec4a vec4a::makez() noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_setzero_ps();
#else
		return {0, 0, 0, 0};
#endif
	}
	template<>
	ATTR_INLINE const vec4a vec4a::makes(float v) noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_set_ss(v);
#else
		return { v, 0, 0, 0 };
#endif
	}
	template<>
	ATTR_INLINE const vec4a vec4a::make(const float * ptr) noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_loadu_ps(ptr);
#else
		return *(vec4a*)ptr;
#endif
	}
	template<>
	ATTR_INLINE void vec4a::get(float * ptr) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		_mm_storeu_ps(ptr, m_vector);
#else
		*(vec4a*)ptr = *this;
#endif
	}
	template<>
	ATTR_INLINE const vec4a vec4a::movs(const vec4a & o) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_move_ss(m_vector, o.m_vector);
#else
		return {o.x, y, z, w};
#endif
	}
	template<>
	ATTR_INLINE const vec4a vec4a::adds(const vec4a & o) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_add_ss(m_vector, o.m_vector);
#else
		return { x + o.x,y,z,w };
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator -() const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return *this ^ (vec4a&)CV_SIGNMASK;
#else
		return { -x, -y, -z, -w };
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator +(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_add_ps(m_vector, v.m_vector);
#else
		return {
			x + v.x,
			y + v.y,
			z + v.z,
			w + v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator -(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_sub_ps(m_vector, v.m_vector);
#else
		return {
			x - v.x,
			y - v.y,
			z - v.z,
			w - v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator *(float value) const noexcept
	{
#ifdef USE_SIMD
		return *this * vec4a(value);
#else
		return {
			x * value,
			y * value,
			z * value,
			w * value
		};
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator /(float value) const noexcept
	{
#ifdef USE_SIMD
		return *this / vec4a(value);
#else
		return {
			x / value,
			y / value,
			z / value,
			w / value
		};
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator *(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_mul_ps(m_vector, v.m_vector);
#else
		return {
			x * v.x,
			y * v.y,
			z * v.z,
			w * v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator /(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_div_ps(m_vector, v.m_vector);
#else
		return {
			x / v.x,
			y / v.y,
			z / v.z,
			w / v.w
		};
#endif
	}
	template <>
	template <>
	ATTR_INLINE const vec4a vec4a::operator *(const mat4a & m) const noexcept
	{
		const float * pos = m_data;
		const vec4a * b = m.v;
		vec4a res;
		res = vec4a(*pos++) * (*b++);
		res += vec4a(*pos++) * (*b++);
		res += vec4a(*pos++) * (*b++);
		res += vec4a(*pos) * (*b);
		return res;
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator &(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_and_ps(m_vector, v.m_vector);
#else
		return ivec4a(
			(int&)x & (int&)v.x,
			(int&)y & (int&)v.y,
			(int&)z & (int&)v.z,
			(int&)w & (int&)v.w
		).fbits();
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator |(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_or_ps(m_vector, v.m_vector);
#else
		return ivec4a(
			(int&)x | (int&)v.x,
			(int&)y | (int&)v.y,
			(int&)z | (int&)v.z,
			(int&)w | (int&)v.w
		).fbits();
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::operator ^(const vec4a & v) const noexcept
	{
#if defined(EMSIMD) || defined(MSSIMD)
		return _mm_xor_ps(m_vector, v.m_vector);
#else
		return ivec4a(
			(int&)x ^ (int&)v.x,
			(int&)y ^ (int&)v.y,
			(int&)z ^ (int&)v.z,
			(int&)w ^ (int&)v.w
		).fbits();
#endif
	}
	template <>
	ATTR_INLINE float vec4a::length_sq() const noexcept
	{
#ifdef USE_SIMD
		return length_sqV().getX();
#else
		return ((vec4*)this)->length_sq();
#endif
	}
	template <>
	ATTR_INLINE float vec4a::length() const noexcept
	{
		return math::sqrt(length_sq());
	}
	template <>
	ATTR_INLINE const vec4a vec4a::length_sqV() const noexcept
	{
#ifdef USE_SIMD
		return dotV(*this, *this);
#else
		return (vec4a)length_sq();
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::lengthV() const noexcept
	{
#ifdef USE_SIMD
		return sqrtv(length_sqV());
#else
		return (vec4a)length();
#endif
	}
	template <>
	ATTR_INLINE const vec4a vec4a::normalize() const noexcept
	{
#ifdef USE_SIMD
		return *this / lengthV();
#else
		return *this / length();
#endif
	}

	template<>
	ATTR_INLINE const ivec4a ivec4a::makez() noexcept
	{
#ifdef MSSIMD
		return _mm_setzero_si128();
#else
		return __ivector4{ 0, 0, 0, 0 };
#endif
	}
	template<>
	ATTR_INLINE const ivec4a ivec4a::makes(int v) noexcept
	{
#ifdef MSSIMD
		return _mm_set_epi32(v, 0, 0, 0);
#else
		return __ivector4{ v, 0, 0, 0 };
#endif
	}
	template<>
	ATTR_INLINE const ivec4a ivec4a::make(const int * ptr) noexcept
	{
#ifdef EMSIMD
		struct unaligned_t {
			__ivector4 __v;
		} __attribute__((__packed__, __may_alias__));

		return ((struct unaligned_t *)ptr)->__v;
#elif defined(MSSIMD)
		return _mm_loadu_si128((__m128i*)ptr);
#else
		return *(ivec4a*)ptr;
#endif
	}
	template<>
	ATTR_INLINE void ivec4a::get(int * ptr) const noexcept
	{
#ifdef EMSIMD
		struct unaligned_t {
			__ivector4 __v;
		} __attribute__((__packed__, __may_alias__));

		((struct unaligned_t *)ptr)->__v = m_vector;
#elif defined(MSSIMD)
		_mm_storeu_si128((__m128i*)ptr, m_vector);
#else
		*(ivec4a*)ptr = *this;
#endif
	}
	template<>
	ATTR_INLINE const ivec4a ivec4a::movs(const ivec4a& o) const noexcept
	{
#ifdef USE_SIMD
		return (o & CV_MASK_X) | ((*this) & CV_MASK_YZW);
#else
		return ivec4a{o.x, y,z,w};
#endif
	}
	template<>
	ATTR_INLINE const ivec4a ivec4a::adds(const ivec4a& o) const noexcept
	{
#ifdef USE_SIMD
		return *this + (o & CV_MASK_X);
#else
		return ivec4a{ x + o.x, y,z,w };
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator -() const noexcept
	{
#ifdef USE_SIMD
		return CV_ZERO - *this;
#else
		return ivec4a{ -x, -y, -z, -w};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator +(const ivec4a & v) const noexcept
	{
#ifdef EMSIMD
		return m_vector + v.m_vector;
#elif defined(MSSIMD)
		return _mm_add_epi32(m_vector, v.m_vector);
#else
		return ivec4a{
			x + v.x,
			y + v.y,
			z + v.z,
			w + v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator -(const ivec4a & v) const noexcept
	{
#ifdef EMSIMD
		return m_vector - v.m_vector;
#elif defined(MSSIMD)
		return _mm_sub_epi32(m_vector, v.m_vector);
#else
		return ivec4a{
			x - v.x,
			y - v.y,
			z - v.z,
			w - v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator *(int value) const noexcept
	{
#ifdef USE_SIMD
		return *this * ivec4a(value);
#else
		return ivec4a{
			x * value,
			y * value,
			z * value,
			w * value
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator /(int value) const noexcept
	{
#ifdef USE_SIMD
		return *this / ivec4a(value);
#else
		return ivec4a{
			x / value,
			y / value,
			z / value,
			w / value
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator *(const ivec4a & v) const noexcept
	{
#ifdef EMSIMD
		return m_vector * v.m_vector;
#elif defined(MSSIMD)
		return _mm_mul_epi32(m_vector, v.m_vector);
#else
		return ivec4a{
			x * v.x,
			y * v.y,
			z * v.z,
			w * v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator /(const ivec4a & v) const noexcept
	{
#ifdef EMSIMD
		return m_vector / v.m_vector;
#elif defined(MSSIMD)
		return ((vec4a)*this / (vec4a)v).itruncate();
#else
		return ivec4a{
			x / v.x,
			y / v.y,
			z / v.z,
			w / v.w
		};
#endif
	}
	template <>
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator *(const imat4a & m) const noexcept
	{
		const int * pos = m_data;
		const ivec4a * b = m.v;
		ivec4a res;
		res = ivec4a(*pos++) * (*b++);
		res += ivec4a(*pos++) * (*b++);
		res += ivec4a(*pos++) * (*b++);
		res += ivec4a(*pos) * (*b);
		return res;
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator &(const ivec4a& v) const noexcept
	{
#ifdef EMSIMD
		return emscripten_int32x4_and(m_vector, v.m_vector);
#elif defined(MSSIMD)
		return _mm_and_si128(m_vector, v.m_vector);
#else
		return ivec4a{
			x & v.x,
			y & v.y,
			z & v.z,
			w & v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator |(const ivec4a& v) const noexcept
	{
#ifdef EMSIMD
		return emscripten_int32x4_or(m_vector, v.m_vector);
#elif defined(MSSIMD)
		return _mm_or_si128(m_vector, v.m_vector);
#else
		return ivec4a{
			x | v.x,
			y | v.y,
			z | v.z,
			w | v.w
		};
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::operator ^(const ivec4a& v) const noexcept
	{
#ifdef EMSIMD
		return emscripten_int32x4_xor(m_vector, v.m_vector);
#elif defined(MSSIMD)
		return _mm_xor_si128(m_vector, v.m_vector);
#else
		return ivec4a{
			x ^ v.x,
			y ^ v.y,
			z ^ v.z,
			w ^ v.w
		};
#endif
	}
	template <>
	ATTR_INLINE int ivec4a::length_sq() const noexcept
	{
#ifdef USE_SIMD
		return length_sqV().getX();
#else
		return ((ivec4*)this)->length_sq();
#endif
	}
	template <>
	ATTR_INLINE int ivec4a::length() const noexcept
	{
#ifdef USE_SIMD
		return math::sqrt(lengthV().getX());
#else
		return ((ivec4*)this)->length();
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::length_sqV() const noexcept
	{
		return dotV(*this, *this);
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::lengthV() const noexcept
	{
#ifdef USE_SIMD
		return sqrtv((vec4a)length_sqV()).iround();
#else
		return (ivec4a)length();
#endif
	}
	template <>
	ATTR_INLINE const ivec4a ivec4a::normalize() const noexcept
	{
#ifdef USE_SIMD
		return *this / lengthV();
#else
		return *this / length();
#endif
	}


	namespace math
	{
		namespace simd
		{
			template <typename T>
			ATTR_INLINE const vector<T, 4, true> dotV(const vector<T, 4, true> & a, const vector<T, 4, true>  & b) noexcept
			{
				using vec = vector<T, 4, true>;
				vec vTemp2 = b;
				vec vTemp = a * vTemp2;
				vTemp2 = vTemp.template shuffle<2, 3, 0, 1>() + vTemp; // 0+2, 1+3, 0+2, 1+3
				return vTemp2.template shuffle<1, 2, 3, 0>() + vTemp2; // 0+1+2+3
			}
			template <typename T>
			ATTR_INLINE const vector<T, 4, true> cross(const vector<T, 4, true> & a, const vector<T, 4, true> & b) noexcept
			{
				using vec = vector<T, 4, true>;

				// x1,y1,z1,w1 - y2,z2,x2,w2
				// x2,y2,z2,w2 - y1,z1,x1,w1
				// (y1*z2-z1*y2), (z1*x2-x1*z2), (x1*y2-y1*x2)

				vec lvMult = b.template shuffle<1, 2, 0, 3>() * a;
				// (x1*y2), (y1*z2), (z1*x2), (w1*w2)

				vec lvMult2 = a.template shuffle<1, 2, 0, 3>() * b;
				// (x2*y1), (y2*z1), (z2*x1), (w2*w1)

				vec res = lvMult - lvMult2;
				return res.template shuffle <1, 2, 0, 3>();
			}
		}
	}
}

template <>
ATTR_INLINE const kr::vec4a dotV<float, 4, true>(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
	return kr::math::simd::dotV(a, b);
}
template <>
ATTR_INLINE const kr::ivec4a dotV<int, 4, true>(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
	return kr::math::simd::dotV(a, b);
}
template <>
ATTR_INLINE const kr::vec4a cross<float, true>(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
	return kr::math::simd::cross(a, b);
}
template <>
ATTR_INLINE const kr::ivec4a cross<int, true>(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
	return kr::math::simd::cross(a, b);
}
template <>
ATTR_INLINE const kr::vec4a operator * <float, 4, true>(float value, const kr::vec4a & b) noexcept
{
	return kr::vec4a(value) * b;
}
template <>
ATTR_INLINE const kr::vec4a operator / <float, 4, true>(float value, const kr::vec4a & b) noexcept
{
	return kr::vec4a(value) / b;
}
template <>
ATTR_INLINE float dot<float, 4, true>(const kr::vec4a & a, const kr::vec4a & b) noexcept
{
	return dotV(a, b).getX();
}
template <>
ATTR_INLINE const kr::ivec4a operator * <int, 4, true>(int value, const kr::ivec4a & b) noexcept
{
	return kr::ivec4a(value) * b;
}
template <>
ATTR_INLINE const kr::ivec4a operator / <int, 4, true>(int value, const kr::ivec4a & b) noexcept
{
	return kr::ivec4a(value) / b;
}
template <>
ATTR_INLINE int dot<int, 4, true>(const kr::ivec4a & a, const kr::ivec4a & b) noexcept
{
	return dotV(a, b).getX();
}
