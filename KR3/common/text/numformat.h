#pragma once

#include <KR3/main.h>
#include <KR3/meta/math.h>

#include "buffer.h"

namespace kr
{
	// 버퍼러블 숫자 (정수)
	template <typename T> class Number :
		public Bufferable<Number<T>, BufferInfo<AutoComponent>>
	{
		static_assert(std::is_arithmetic<T>::value, "T is not number");
	private:
		std::make_unsigned_t<T> m_value;
		uint m_radix;
		dword m_cipher;
		bool m_minus;
		size_t m_size;

	public:
		Number(T _value, uint _radix = 10) noexcept
		{
			m_radix = _radix;
			m_cipher = math::cipher(_value, m_radix);
			m_minus = _value < 0;
			if (m_minus)
			{
				m_value = -std::make_signed_t<T>(_value);
			}
			else
			{
				m_value = _value;
			}
			m_size = (size_t)m_cipher + m_minus;
		}

		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return m_size;
		}
		template <typename C>
		size_t $copyTo(C * dest) const noexcept
		{
			if (m_minus) *dest++ = '-';
			memt<sizeof(C)>::fromint(dest, m_cipher, m_value, m_radix);
			return m_size;
		}
	};

	// 버퍼러블 숫자 (정수)
	// 가로 폭을 고정시킬 수 있다.
	template <typename T> class NumberFixed : 
		public Bufferable<NumberFixed<T>, BufferInfo<AutoComponent> >
	{
		static_assert(std::is_arithmetic<T>::value, "T is not number");
	private:
		size_t m_size;
		T m_value;
		dword m_radix;
		dword m_contlen;
		dword m_zerolen;
		char m_fillchr;
		bool m_minus;

	public:
		NumberFixed(T _value, dword _radix, dword _fixed, char fillchr) noexcept
		{
			m_minus = _value < 0;
			if (m_minus)
			{
				m_value = -std::make_signed_t<T>(_value);
			}
			else
			{
				m_value = _value;
			}
			m_fillchr = fillchr;
			m_radix = _radix;
			dword cipher = math::cipher(m_value, m_radix);
			m_contlen = cipher;
			m_zerolen = maxt(cipher, _fixed) - cipher;
			m_size = (size_t)m_contlen + m_zerolen + m_minus;
		}

		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return m_size;
		}
		template <typename C>
		size_t $copyTo(C * dest) const noexcept
		{
			if (m_minus) *dest++ = '-';

			using memm = memt<sizeof(C)>;
			memm::set(dest, m_fillchr, m_zerolen);
			memm::fromint(dest + m_zerolen, m_contlen, m_value, m_radix);
			return m_size;
		}
	};

	// 버퍼러블 숫자 (포인터)
	class NumberAddress: public Bufferable<NumberAddress, BufferInfo<AutoComponent> >
	{
	private:
		uintptr_t m_value;

	public:
		NumberAddress(const void* _value) noexcept
		{
			m_value = (uintptr_t)_value;
		}

		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return sizeof(void*) * 2 + 2;
		}
		template <typename C>
		size_t $copyTo(C * dest) const noexcept
		{
			*dest++ = (C)'0';
			*dest++ = (C)'x';
			memt<sizeof(C)>::fromint(dest, sizeof(void*) * 2, m_value, 16);
			return $sizeAs<C>();
		}
	};

	// 버퍼러블 숫자 (실수)
	// FIXED: 표시할 소수점 수, 0으로 하면 자동으로 된다.
	template <dword FIXED, typename T>
	class NumberFloat
		:public Bufferable<NumberFloat<FIXED>, BufferInfo<AutoComponent>>
	{
	private:
		size_t m_size;
		dword m_integer;
		dword m_decimal;
		dword m_cipher;
		dword m_zerolen;
		bool m_minus;

	public:
		static constexpr dword fixed = FIXED;

		NumberFloat(T v) noexcept
		{
			m_minus = v < 0;
			if (m_minus)
				v = -v;

			constexpr dword decfixed = meta::pow(10UL, FIXED);
			dword value = lround(v * decfixed);
			m_integer = value / decfixed;
			m_decimal = value % decfixed;

			m_zerolen = FIXED - math::cipher(m_decimal, 10);
			m_cipher = math::cipher(m_integer, 10);
			m_size = m_cipher + FIXED + m_minus + 1;
		}

		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return m_size;
		}
		template <typename C>
		size_t $copyTo(C *dest) const noexcept
		{
			using memm = memt<sizeof(C)>;
			if (m_minus)
				*dest++ = '-';
			memm::fromint(dest, m_cipher, m_integer, 10);
			dest += m_cipher;
			*dest++ = '.';
			memm::set(dest, '0', m_zerolen);
			memm::fromint(dest, FIXED, m_decimal, 10);
			return m_size;
		}
	};

	// 버퍼러블 숫자 (실수)
	// FIXED: 표시할 소수점 수, 0으로 하면 자동으로 된다.
	template <typename T>
	class NumberFloat<0, T>
		:public AddBufferable<NumberFloat<0, T>, BufferInfo<AutoComponent>>
	{
	private:
		enum class Mode
		{
			ExpMinus,
			ExpPlus,
			Decimal,
			Integer,
			Mixed,
			Infinite,
			NaN
		};
		union
		{
			struct
			{
				int exp;
				int explen;
				dword decimal;
			} m_exp;
			struct
			{
				dword value;
			} m_decimal;
			struct
			{
				dword value;
			} m_integer;
			struct
			{
				dword pointpos;
				dword integer;
				dword decimal;
			} m_mixed;
		};
		bool m_minus;
		Mode m_mode;
		size_t m_size;

	public:
		static constexpr dword fixed = 0;
		static constexpr int max_size = 9;

		NumberFloat(T v) noexcept
		{
			if (!isfinite(v))
			{
				if (isinf(v))
				{
					m_minus = (v < 0);
					m_mode = Mode::Infinite;
					m_size = (size_t)m_minus + 3;
				}
				else
				{
					m_minus = false;
					m_mode = Mode::NaN;
					m_size = 3;
				}
				return;
			}
			m_minus = v < 0;
			if (m_minus)
				v = -v;

			if (v != 0)
			{
				T floored = floor(log10(v));
				int len = (int)floored;
				if (len < -max_size + 4)
				{
					m_exp.exp = -len;
					m_exp.explen = math::cipher(m_exp.exp, 10);
					int declen_approxy = max_size - 4 - m_exp.explen;
					if (declen_approxy < 3) declen_approxy = 3;
					int decimal = (dword)lround(v * pow((T)10, declen_approxy - floored));
					int declen = math::cipher(decimal, 10);
					int size = declen + m_exp.explen + 2; // negative sign & e
					while (decimal % 10 == 0)
					{
						decimal /= 10;
						size--;
					}
					if (decimal >= 10) size++; // dot
					m_exp.decimal = decimal;
					m_mode = Mode::ExpMinus;
					m_size = size;
				}
				else if (len < 0)
				{
					dword value = (dword)lround(v * pow((T)10, max_size - 2));
					dword size = max_size;
					for (;;)
					{
						if (value % 10 != 0)
							break;
						value /= 10;
						size--;
					}
					m_size = size;
					m_mode = Mode::Decimal;
					m_decimal.value = value;
				}
				else if (len < max_size - 2)
				{
					int pointpos = -len + max_size - 2;
					dword axis = 1;
					for (int i = 0; i < pointpos; i++)
						axis *= 10;
					dword value = (dword)lround(v * (T)axis);
					dword decimal = value % axis;
					dword integer = value / axis;
					if (decimal == 0)
					{
						m_integer.value = integer;
						m_mode = Mode::Integer;
						m_size = (size_t)len + 1;
					}
					else
					{
						m_mode = Mode::Mixed;
						dword size = max_size;
						m_mixed.integer = integer;
						for (;;)
						{
							if (decimal % 10 != 0)
								break;
							decimal /= 10;
							size--;
							pointpos--;
						}
						m_mixed.pointpos = pointpos;
						m_mixed.decimal = decimal;
						m_size = size;
					}
				}
				else if (len < max_size)
				{
					m_mode = Mode::Integer;
					m_integer.value = (dword)lround(v);
					m_size = (size_t)len + 1;
				}
				else
				{
					m_exp.exp = len;
					m_exp.explen = math::cipher(m_exp.exp, 10);
					int declen_approxy = max_size - 3 - m_exp.explen;
					if (declen_approxy < 3) declen_approxy = 3;
					int decimal = (dword)lround(v * pow((T)10, -floored + declen_approxy));
					int declen = math::cipher(decimal, 10);
					int size = declen + m_exp.explen + 1; // e
					while (decimal % 10 == 0)
					{
						decimal /= 10;
						size--;
					}
					if (decimal >= 10) size++; // dot
					m_exp.decimal = decimal;
					m_mode = Mode::ExpPlus;
					m_size = size;
				}
			}
			else
			{
				m_mode = Mode::Integer;
				m_integer.value = 0;
				m_size = 1;
			}
			m_size += m_minus;
		}

		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return m_size;
		}
		template <typename C>
		size_t $copyTo(C *dest) const noexcept
		{
			C * end = dest + m_size;
			if (m_minus)
				*dest++ = '-';

			using memm = memt<sizeof(C)>;

			switch (m_mode)
			{
			case Mode::ExpPlus:
			case Mode::ExpMinus:
			{
				dword value = m_exp.exp;
				C * to = end - m_exp.explen;
				_assert(to <= end);
				while (end != to)
				{
					*--end = (C)(value % 10 + '0');
					value /= 10;
				}
				if (m_mode == Mode::ExpMinus) *--end = (C)'-';
				*--end = (C)'e';
				value = m_exp.decimal;
				if (m_exp.decimal < 10)
				{
					*dest = (C)(value + '0');
				}
				else
				{
					to = dest + 2; // major and dot
					_assert(to <= end);
					while (end != to)
					{
						end--;
						*end = (C)(value % 10 + '0');
						value /= 10;
					}
					_assert(dest == to - 2);
					dest[0] = (C)(value + '0');
					dest[1] = (C)'.';
				}
				break;
			}
			case Mode::Decimal:
			{
				dword value = m_decimal.value;
				C * beg = dest + 2;
				while (end != beg)
				{
					end--;
					*end = (C)((value % 10) + '0');
					value /= 10;
				}
				end--;
				*end = '.';
				end--;
				*end = '0';
				break;
			}
			case Mode::Integer:
			{
				dword value = m_integer.value;
				while (end != dest)
				{
					end--;
					*end = (C)(value % 10 + '0');
					value /= 10;
				}
				break;
			}
			case Mode::Mixed:
			{
				dword value = m_mixed.decimal;
				C * to = end - m_mixed.pointpos;
				while (end != to)
				{
					end--;
					*end = (C)(value % 10 + '0');
					value /= 10;
				}
				end--;
				*end = '.';
				value = m_mixed.integer;
				while (end != dest)
				{
					end--;
					*end = (C)(value % 10 + '0');
					value /= 10;
				}
				break;
			}
			case Mode::Infinite:
				*dest++ = 'i';
				*dest++ = 'n';
				*dest++ = 'f';
				break;
			case Mode::NaN:
				*dest++ = 'n';
				*dest++ = 'a';
				*dest++ = 'n';
				break;
			}
			return m_size;
		}
	};


	template <typename Component> 
	class Fill :public Bufferable<Fill<Component>, BufferInfo<Component>>
	{
	public:
		const Component value;
		const size_t count;

		Fill(Component _value, size_t _count) noexcept
			: value(_value)
			, count(_count)
		{
		}

		size_t $size() const noexcept
		{
			return count;
		}
		size_t $copyTo(Component* dest) const noexcept
		{
			mema::ctor_fill(dest, value, count);
			return count;
		}
	};

	template <typename Component, typename Text>
	class FillLeft :public Bufferable<FillLeft<Component, Text>, BufferInfo<Component>>
	{
	public:
		const Component value;
		const Text& m_text;
		size_t totalSize;
		size_t count;

		FillLeft(Component _value, size_t _count, const Text & text) noexcept
			: value(_value)
			, m_text(text)
		{
			size_t txsize = m_text.template sizeAs<Component>();
			totalSize = maxt(txsize, _count);
			count = totalSize - txsize;
		}

		size_t $size() const noexcept
		{
			return totalSize;
		}
		size_t $copyTo(Component* dest) const noexcept
		{
			mema::ctor_fill(dest, value, count);
			m_text.copyTo(dest + count);
			return totalSize;
		}
	};

	class CostNumber : public Bufferable<CostNumber, BufferInfo<AutoComponent>>
	{
	private:
		size_t m_size;
		uint m_number;
		uint m_radix;

	public:
		CostNumber(uint number, uint radix = 10) noexcept
		{
			m_radix = radix;
			uint cipher = math::cipher(number, radix);
			m_size = (size_t)cipher + (cipher - 1) / 3;
			m_number = number;
		}

		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return m_size;
		}
		template <typename C>
		size_t $copyTo(C * dest) const noexcept
		{
			dest += m_size;
			uint counter = 3;
			uint num = m_number;
			for (;;)
			{
				*--dest = mem::inttochr(num % m_radix);
				num /= m_radix;
				if (num == 0) break;
				counter--;
				if (counter == 0)
				{
					counter = 3;
					*--dest = ',';
				}
			}
			return m_size;
		}
	};

	namespace _pri_
	{
		template <typename T>
		inline Number<T> numberBufferize(T v) { return{ v, 10 }; }
		inline NumberFloat<0> numberBufferize(float v) { return{ v }; }
		inline NumberFloat<0, double> numberBufferize(double v) { return{ v }; }
		inline NumberFloat<0, long double> numberBufferize(long double v) { return{ v }; }
	}
	
	// 부동 소수점을 정해진 숫자의 소수점까지 보이게 한다.
	// FIXED: 표시할 소수점 수
	template <dword FIXED>
	inline NumberFloat<FIXED> pointf(float v)
	{
		return{ v };
	};

	// 부동 소수점을 정해진 숫자의 소수점까지 보이게 한다.
	// FIXED: 표시할 소수점 수
	template <dword FIXED>
	inline NumberFloat<FIXED, double> pointf(double v)
	{
		return{ v };
	};

	// 부동 소수점을 정해진 숫자의 소수점까지 보이게 한다.
	// FIXED: 표시할 소수점 수
	template <dword FIXED>
	inline NumberFloat<FIXED, long double> pointf(long double v)
	{
		return{ v };
	};

	// 숫자를 10진수로 보이게 한다.
	// 그냥 출력하는 것과 동일하다.
	template <typename T> Number<T> decf(T _v)
	{
		return{ _v, 10 };
	};

	// 숫자를 16진수로 보이게 한다.
	template <typename T> Number<T> hexf(T _v)
	{
		return{ _v, 16 };
	};

	// 숫자를 몇진수로 띄울 지 정한다.
	// _v: 출력할 숫자
	// _radix: 진수
	template <typename T> Number<T> radixf(T _v, dword _radix)
	{
		return{ _v, _radix };
	};

	// 숫자에 출력 포맷을 정한다. (16진수)
	// _v: 출력할 숫자
	// _fixed: 숫자가 차지할 길이
	// _fillchr: 빈 공간에 채울 글자
	template <typename T> NumberFixed<T> hexf(T _v, dword _fixed, char _fillchr = '0')
	{
		return{ _v, 16, _fixed, _fillchr };
	};

	// 숫자에 출력 포맷을 정한다. (10진수)
	// _v: 출력할 숫자
	// _fixed: 숫자가 차지할 길이
	// _fillchr: 빈 공간에 채울 글자
	template <typename T> NumberFixed<T> decf(T _v, dword _fixed, char _fillchr = '0')
	{
		return{ _v, 10, _fixed, _fillchr };
	};

	// 숫자에 출력 포맷을 정한다.
	// _v: 출력할 숫자
	// _radix: 진수
	// _fixed: 숫자가 차지할 길이
	// _fillchr: 빈 공간에 채울 글자
	template <typename T> NumberFixed<T> radixf(T _v, dword _radix, dword _fixed, char _fillchr = '0')
	{
		return{ _v, _radix, _fixed, _fillchr };
	};

	template <typename T> Fill<T> fill(T v, size_t sz) noexcept
	{
		return Fill<T>(v, sz);
	};

	template <typename T, typename Text> FillLeft<T, Text> fill(T v, size_t sz, const Text & text) noexcept
	{
		return FillLeft<T, Text>( v, sz, text );
	};
	
}