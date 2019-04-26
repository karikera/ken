#pragma once

#include <cmath>

#pragma optimize("ts",on)

namespace kr
{

	namespace math
	{
		namespace _pri_
		{
			template <int i> struct nearest_float_type_from_size{};
			template <> struct nearest_float_type_from_size<0>{	using type = float;			};
			template <> struct nearest_float_type_from_size<1>{	using type = double;		};
			template <> struct nearest_float_type_from_size<2>{	using type = long double;	};

			template <typename T> using nearest_float = typename nearest_float_type_from_size <
				meta::bool_index<sizeof(T) <= 4, sizeof(T) <= 8, true>::value
			> :: type;
		}
	}
}

template <typename T, typename ... ARGS>
T kr::tmax(T arg, ARGS ... args) noexcept
{
	T out = arg;
	unpack(args > out ? (out = args) : 0);
	return out;
}

template <typename T, typename ... ARGS>
T kr::tmin(T arg, ARGS ... args) noexcept
{
	T out = arg;
	unpack(args < out ? (out = args) : 0);
	return out;
}

template <typename T>
kr::SinCosT<T>::SinCosT(T sin, T cos) noexcept
	:sin(sin), cos(cos)
{
}
template <typename T>
template <typename T2>
kr::SinCosT<T>::SinCosT(const SinCosT<T2>& other) noexcept
	:sin((T)other.sin), cos((T)other.cos)
{
}

template <typename T> T kr::math::pow(T base, T exp) noexcept
{
    T result = 1;
	for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
		if (exp == 0) return result;
        base *= base;
    }
	return result;
}
template <typename T> T kr::math::powi(T base, int exp) noexcept
{
	uint tester;
	if (exp >= 0)
		tester = static_cast<T>(exp);
	else
		tester = static_cast<T>(-exp);

	for (T result = static_cast<T>(1);; base *= base)
	{
		if ((tester & 1) != 0)
			result *= base;
		if ((tester >>= 1) == 0)
			return (exp < 0 ? static_cast<T>(1) / result : result);
	}
}
template <typename T> T kr::math::sqrt(T x) noexcept
{
	return (T)std::sqrt((_pri_::nearest_float<T>)x);
}
template <typename T> T kr::math::abs(T a) noexcept
{
	return (a > 0 ? a : -a);
}
template <typename T>
bool kr::math::between(T min, T value, T max) noexcept
{
	return min <= value && value <= max;
}
template <typename T> T kr::math::getsign(T a) noexcept
{
	return (a < 0) ? -1 : 1;
}
template <typename T> T kr::math::getsigni(T a) noexcept
{
	return (a < 0) ? 1 : -1;
}
template <typename T2, typename T> T2 kr::math::addc(T* _res, T _a, T _b) noexcept
{
	static_assert(sizeof(T) > 8, "Not Supported Size");
	return intrinsic<8>::adc(0, _a, _b, _res);
}
template <typename T2, typename T> T2 kr::math::subb(T* _res, T _a, T _b) noexcept
{
	static_assert(sizeof(T) > 8, "Not Supported Size");
	return intrinsic<8>::sbb(0, _a, _b, _res);
}
template <typename T> int kr::math::suspectedFloor(T value) noexcept
{
	constexpr int BIG_ENOUGH_VALUE  = 16 * 1024;
	return (int)(value+BIG_ENOUGH_VALUE) - BIG_ENOUGH_VALUE;
}
template <typename T>
T kr::math::sin_t(T a) noexcept
{
	return (T)sin((float)a);
}
template <typename T>
T kr::math::cos_t(T a) noexcept
{
	return (T)cos((float)a);
}
template <typename T>
kr::SinCosT<T> kr::math::sincos_t(T a) noexcept
{
	return (SinCosT<T>)sincos((float)a);
}
template <>
ATTR_INLINE double kr::math::sin_t<double>(double a) noexcept
{
	return ::sin(a);
}
template <>
ATTR_INLINE double kr::math::cos_t<double>(double a) noexcept
{
	return ::cos(a);
}
template <>
ATTR_INLINE kr::SinCosT<double> kr::math::sincos_t<double>(double a) noexcept
{
	return { ::sin(a), ::cos(a) };
}
template <>
ATTR_INLINE long double kr::math::sin_t<long double>(long double a) noexcept
{
	return ::sinl(a);
}
template <>
ATTR_INLINE long double kr::math::cos_t<long double>(long double a) noexcept
{
	return ::cosl(a);
}
template <>
ATTR_INLINE kr::SinCosT<long double> kr::math::sincos_t<long double>(long double a) noexcept
{
	return {::sinl(a), ::cosl(a)};
}
template <typename T> kr::dword kr::math::cipher(T n_number, uint n_radix) noexcept
{
	uint cipher = 0;

	if (std::is_unsigned<T>::value)
	{
		do
		{
			cipher++;
			n_number /= n_radix;
		}
		while (n_number != 0);
	}
	else
	{
		_assert(n_radix <= 0x7fffffff);
		do
		{
			cipher++;
			n_number /= (int)n_radix;
		}
		while (n_number != 0);
	}

	return cipher;
}
template <typename T> T kr::math::tround(float x) noexcept
{
	return (T)std::lroundf(x);
}

template <typename T>
T kr::math::mod(T a, T b) noexcept
{
	return (T)(a % b);
}

#pragma optimize("",on)