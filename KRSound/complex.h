#pragma once

namespace kr
{
	struct complex
	{
		float real;
		float imag;

		complex() = default;
		complex(float real, float imag) noexcept;
		explicit complex(float real) noexcept;
		const complex operator +(const complex & other) const noexcept;
		const complex operator -(const complex & other) const noexcept;
		const complex operator *(const complex & other) const noexcept;
		const complex operator /(const complex & other) const noexcept;
		const complex operator *(float other) const noexcept;
		const complex operator /(float other) const noexcept;
		complex& operator +=(const complex & other) noexcept;
		complex& operator -=(const complex & other) noexcept;
		complex& operator *=(const complex & other) noexcept;
		complex& operator /=(const complex & other) noexcept;
		complex& operator *=(float other) noexcept;
		complex& operator /=(float other) noexcept;
		explicit operator float() const noexcept;
		float length() const noexcept;
	};

	struct complex_ex:private complex
	{
		using complex::real;
		using complex::imag;
		float length;

		complex_ex() = default;
		complex_ex(float real, float imag) noexcept;
		complex_ex(float real, float imag, float length) noexcept;
		explicit complex_ex(float real) noexcept;
		explicit complex_ex(const complex & other) noexcept;
		const complex project() const noexcept;
		const complex_ex operator +(const complex_ex & other) const noexcept;
		const complex_ex operator -(const complex_ex & other) const noexcept;
		const complex_ex operator *(float other) const noexcept;
		const complex_ex operator /(float other) const noexcept;
		complex_ex& operator +=(const complex_ex & other) noexcept;
		complex_ex& operator -=(const complex_ex & other) noexcept;
		complex_ex& operator *=(float other) noexcept;
		complex_ex& operator /=(float other) noexcept;
	};

}
