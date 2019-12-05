#pragma once

#include "../meta/boolidx.h"

namespace kr
{
	template <typename T>
	struct SinCosT
	{
		T sin, cos;

		SinCosT() = default;
		SinCosT(T sin, T cos) noexcept;
		template <typename T2>
		SinCosT(const SinCosT<T2>& other) noexcept;
	};
	using SinCos = SinCosT<float>;

#define _RETURN_	ATTR_NO_DISCARD ATTR_NOALIAS

	template <typename T, typename ... ARGS>
	_RETURN_ T maxt(T arg, ARGS ... args) noexcept;
	template <typename T, typename ... ARGS>
	_RETURN_ T mint(T arg, ARGS ... args) noexcept;
	template <typename T, typename ... ARGS>
	_RETURN_ T sumt(T arg, ARGS ... args) noexcept;
	template <typename T>
	_RETURN_ T clampt(T _min, T _v, T _max) noexcept;

	namespace math
	{
		constexpr float pi = 3.141592653589f;
		constexpr double pid = 3.14159265358979323846;
		constexpr float tau = pi*2.f;
		constexpr double taud = pid*2.0;

		template <typename T> _RETURN_ T pow(T base, T exp) noexcept;
		template <> _RETURN_ float pow<float>(float x, float y) noexcept;
		template <> _RETURN_ double pow<double>(double x, double y) noexcept;

		template <typename T> _RETURN_ T powi(T base, int exp) noexcept;
		template <typename T> _RETURN_ T sqrt(T x) noexcept;
		template <typename T> _RETURN_ T abs(T a) noexcept;
		template <> _RETURN_ float abs<float>(float a) noexcept;
		template <> _RETURN_ double abs<double>(double a) noexcept;
		template <typename T>
		_RETURN_ bool between(T min, T value, T max) noexcept;
		template <typename T> _RETURN_ T getsign(T a) noexcept;
		template <typename T> _RETURN_ T getsigni(T a) noexcept;
		template <typename T2, typename T> _RETURN_ T2 addc(T* _res, T _a, T _b) noexcept;
		template <typename T2, typename T> _RETURN_ T2 subb(T* _res, T _a, T _b) noexcept;
		template <typename T> _RETURN_ int suspectedFloor(T value) noexcept;

		_RETURN_ uint32_t pow2ceil(uint32_t number) noexcept;
		_RETURN_ uint32_t pow2floor(uint32_t number) noexcept;
		template <typename T> _RETURN_ uint32_t cipher(T n_number, uint n_radix) noexcept;
		_RETURN_ float sin(float a) noexcept;
		_RETURN_ float cos(float a) noexcept;
		SinCos sincos(float a) noexcept;
		template <typename T>
		_RETURN_ T sin_t(T a) noexcept;
		template <typename T>
		_RETURN_ T cos_t(T a) noexcept;
		template <typename T>
		SinCosT<T> sincos_t(T a) noexcept;
		_RETURN_ float tan(float a) noexcept;
		_RETURN_ float asin(float a) noexcept;
		_RETURN_ float acos(float a) noexcept;
		_RETURN_ float atan(float a) noexcept;
		_RETURN_ float atan2(float y, float x) noexcept;
		_RETURN_ uint32_t gap(uint32_t a, uint32_t b) noexcept;
		_RETURN_ float ceil(float a) noexcept;
		_RETURN_ uint32_t ceil(uint32_t a, uint32_t b) noexcept;
		_RETURN_ float ceil(float a, float b) noexcept;
		_RETURN_ float round(float x) noexcept;
		_RETURN_ uint32_t lround(float x) noexcept;
		template <typename T> _RETURN_ T tround(float x) noexcept;
		template <> _RETURN_ float tround<float>(float x) noexcept;
		template <> _RETURN_ double tround<double>(float x) noexcept;
		template <> _RETURN_ unsigned long long tround<unsigned long long>(float x) noexcept;
		template <> _RETURN_ long long tround<long long>(float x) noexcept;
		_RETURN_ float floor(float a) noexcept;
		template <typename T>
		_RETURN_ T mod(T a, T b) noexcept;
		template <>
		_RETURN_ float mod<float>(float a, float b) noexcept;
		template <>
		_RETURN_ double mod<double>(double a, double b) noexcept;
		_RETURN_ float radianmod(float a) noexcept;
		_RETURN_ float uradianmod(float a) noexcept;
		_RETURN_ double radianmod(double a) noexcept;
		_RETURN_ double uradianmod(double a) noexcept;
		_RETURN_ uint32_t ilog(uint32_t val, uint32_t n) noexcept;
		_RETURN_ uint32_t ilog2(uint32_t val) noexcept;
		_RETURN_ uint32_t plog2(uintptr_t val) noexcept;

		#undef _RETURN_

	}

}

#include "math.inl"
