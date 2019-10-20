#pragma once

#ifndef __KR3_INCLUDED

namespace kr
{
	using char16 = char16_t;
	using pcstr16 = const char16*;
	using byte = uint8_t;


	struct Empty;

	template <typename C, bool, bool, typename Parent = Empty>
	class BufferInfo;

	struct Empty
	{
	};

	template <typename T = Empty>
	class Interface : public T
	{
	public:
		void AddRef() noexcept = delete;
		int Release() noexcept = delete;
	};

	namespace _pri_
	{
		inline int ipow(int base, int exp) noexcept
		{
			int result = 1;
			for (;;)
			{
				if (exp & 1)
					result *= base;
				exp >>= 1;
				if (!exp)
					break;
				base *= base;
			}

			return result;
		}

		inline int to_int(pcstr16 p, pcstr16 pend) noexcept
		{
			bool minus;
			char16 chr;


			for (;;)
			{
				if (p == pend) return 0;
				chr = *p++;
				if (chr == ' ')
				{
					continue;
				}
				break;
			}

			pend--;
			for (;;)
			{
				if (*pend == ' ')
				{
					if (pend == p) return 0;
					pend--;
					continue;
				}
				break;
			}
			pend++;

			for (;;)
			{
				if (chr == '-')
				{
					if (p == pend) return 0;
					chr = *p++;
					minus = true;
					break;
				}
				minus = false;
				break;
			}

			if ('0' > chr || chr > '9')
			{
				return 0;
			}

			int number;

			if (chr == '0')
			{
				if (p == pend) return 0;

				chr = *p++;
				if (chr == 'x' || chr == 'X')
				{
					goto _hex;
				}
				number = 0;
			}
			else
			{
				number = chr - '0';
				chr = *p++;
			}

			for (;;)
			{
				if ('0' <= chr && chr <= '9')
				{
					number *= 10;
					number += chr - '0';
				}
				else if (chr == 'e')
				{
					goto _exp;
				}
				else
				{
					return 0;
				}
				if (p == pend) return minus ? -number : number;
				chr = *p++;
			}

		_hex:
			if (p == pend) return 0;
			number = 0;
			for (;;)
			{
				chr = *p++;
				if ('0' <= chr && chr <= '9')
				{
					number |= (chr - '0');
				}
				else if ('A' <= chr && chr <= 'Z')
				{
					number |= (chr - ('A' - 10));
				}
				else if ('a' <= chr && chr <= 'z')
				{
					number |= (chr - ('a' - 10));
				}
				else
				{
					return 0;
				}
				if (p == pend)
				{
					return minus ? -number : number;
				}
				number <<= 4;
			}


		_exp:
			if (p == pend) return 0;
			for (;;)
			{
				int exp = 0;

				chr = *p++;
				if ('0' <= chr && chr <= '9')
				{
					exp += chr - '0';
				}
				if (p == pend)
				{
					number *= _pri_::ipow(10, exp);
					return minus ? -number : number;
				}
				exp *= 10;
			}

		}

		inline float to_float(pcstr16 p, pcstr16 pend) noexcept
		{
			bool minus;
			char16 chr;

			for (;;)
			{
				if (p == pend) return 0;
				chr = *p++;
				if (chr == ' ')
				{
					continue;
				}
				break;
			}

			pend--;
			for (;;)
			{
				if (*pend == ' ')
				{
					if (pend == p) return 0;
					pend--;
					continue;
				}
				break;
			}
			pend++;

			for (;;)
			{
				if (chr == '-')
				{
					if (p == pend) return 0;
					chr = *p++;
					minus = true;
					break;
				}
				minus = false;
				break;
			}

			if ('0' > chr || chr > '9')
			{
				return 0;
			}

			float number = (float)(chr - '0');
			for (;;)
			{
				if ('0' <= chr && chr <= '9')
				{
					number *= 10.f;
					number += (float)(chr - '0');
				}
				else if (chr == '.')
				{
					goto _dec;
				}
				else if (chr == 'e')
				{
					goto _exp;
				}
				else
				{
					return 0;
				}
				if (p == pend) return minus ? -number : number;
				chr = *p++;
			}

		_dec:
			float dec = 0.1f;
			for (;;)
			{
				if ('0' <= chr && chr <= '9')
				{
					number += (float)(chr - '0') * dec;
				}
				else if (chr == 'e')
				{
					goto _exp;
				}
				else
				{
					return 0;
				}
				if (p == pend) return minus ? -number : number;
				chr = *p++;
				dec *= 0.1f;
			}

		_exp:
			if (p == pend) return 0;
			for (;;)
			{
				float exp = 0;

				chr = *p++;
				if ('0' <= chr && chr <= '9')
				{
					exp += (float)(chr - '0');
				}
				if (p == pend)
				{
					number *= powf(10.f, exp);
					return minus ? -number : number;
				}
				exp *= 10.f;
			}
		}
	}


	template <typename Self, typename BufferInfo>
	class Bufferable;

	template <typename Self, typename C, bool readonly, typename Parent>
	class Bufferable<Self, BufferInfo<C, readonly, true, Parent> >
	{
	private:
		const C* _begin() const noexcept
		{
			return static_cast<const Self*>(this)->begin();
		}
		const C* _end() const noexcept
		{
			return static_cast<const Self*>(this)->end();
		}
		size_t _size() const noexcept
		{
			return static_cast<const Self*>(this)->size();
		}

	public:
		int to_int() const noexcept
		{
			return _pri_::to_int(_begin(), _end());
		}
		
		float to_float() const noexcept
		{
			return _pri_::to_float(_begin(), _end());
		}
	};

	template <typename C, bool, bool, typename Parent>
	class BufferInfo:public Parent
	{
	};

	using std::move;
}

#endif