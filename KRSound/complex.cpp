#include "stdafx.h"
#include "complex.h"

#include <math.h>

using namespace kr;

complex::complex(float real, float imag) noexcept
	:real(real), imag(imag)
{
}
complex::complex(float real) noexcept
	:real(real), imag(0.f)
{
}
const complex complex::operator +(const complex & other) const noexcept
{
	return { real + other.real, imag + other.imag };
}
const complex complex::operator -(const complex & other) const noexcept
{
	return { real - other.real, imag - other.imag };
}
const complex complex::operator *(const complex & other) const noexcept
{
	return { real * other.real - imag * other.imag, real * other.imag + imag * other.real };
}
const complex complex::operator /(const complex & other) const noexcept
{
	return { real / other.real - imag / other.imag, real / other.imag + imag / other.real };
}
const complex complex::operator *(float other) const noexcept
{
	return { real * other, imag * other };
}
const complex complex::operator /(float other) const noexcept
{
	return { real / other, imag / other };
}
complex& complex::operator +=(const complex & other) noexcept
{
	real += other.real;
	imag += other.imag;
	return *this;
}
complex& complex::operator -=(const complex & other) noexcept
{
	real -= other.real;
	imag -= other.imag;
	return *this;
}
complex& complex::operator *=(const complex & other) noexcept
{
	float treal = real;
	real = treal * other.real - imag * other.imag;
	imag = treal * other.imag + imag * other.real;
	return *this;
}
complex& complex::operator /=(const complex & other) noexcept
{
	float treal = real;
	real = treal / other.real - imag / other.imag;
	imag = treal / other.imag + imag / other.real;
	return *this;
}
complex& complex::operator *=(float other) noexcept
{
	real *= other;
	imag *= other;
	return *this;
}
complex& complex::operator /=(float other) noexcept
{
	real /= other;
	imag /= other;
	return *this;
}
float complex::length() const noexcept
{
	return sqrtf(real * real + imag * imag);
}

complex_ex::complex_ex(float real, float imag) noexcept
	:complex(real, imag)
{
	length = complex::length();
}
complex_ex::complex_ex(float real, float imag, float length) noexcept
	: complex(real, imag), length(length)
{
}
complex_ex::complex_ex(float real) noexcept
	:complex(real)
{
	length = real;
}
complex_ex::complex_ex(const complex & other) noexcept
	:complex(other)
{
	length = complex::length();
}
const complex complex_ex::project() const noexcept
{
	float scale = complex::length();
	if (scale < 0.0001f) return {real, imag};
	scale = length / scale;
	return {real * scale, imag * scale };
}
const complex_ex complex_ex::operator +(const complex_ex & other) const noexcept
{
	return { real + other.real, imag + other.imag, length + other.length };
}
const complex_ex complex_ex::operator -(const complex_ex & other) const noexcept
{
	return { real - other.real, imag - other.imag, length - other.length };
}
const complex_ex complex_ex::operator *(float other) const noexcept
{
	return { real * other, imag * other, length * other };
}
const complex_ex complex_ex::operator /(float other) const noexcept
{
	return { real / other, imag / other, length / other };
}
complex_ex& complex_ex::operator +=(const complex_ex & other) noexcept
{
	real += other.real;
	imag += other.imag;
	length += other.length;
	return *this;
}
complex_ex& complex_ex::operator -=(const complex_ex & other) noexcept
{
	real -= other.real;
	imag -= other.imag;
	length -= other.length;
	return *this;
}
complex_ex& complex_ex::operator *=(float other) noexcept
{
	real *= other;
	imag *= other;
	length *= other;
	return *this;
}
complex_ex& complex_ex::operator /=(float other) noexcept
{
	real *= other;
	imag *= other;
	length *= other;
	return *this;
}
