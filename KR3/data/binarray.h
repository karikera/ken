#pragma once

#include "../main.h"
#include "../meta/log2.h"

namespace kr
{

	template <typename Derived>
	class BinArrayMethod
	{
	private:
		uintptr_t * _data() noexcept
		{
			return static_cast<Derived*>(this)->data();
		}
		const uintptr_t * _data() const noexcept
		{
			return static_cast<const Derived*>(this)->data();
		}
		size_t _sizep() const noexcept
		{
			return static_cast<const Derived*>(this)->sizep();
		}
		size_t _size() const noexcept
		{
			return static_cast<const Derived*>(this)->size();
		}

	public:
		static constexpr size_t bitsize = sizeof(uintptr_t) * 8;
		static constexpr size_t bitmask = bitsize - 1;
		static constexpr size_t bitshift = meta::ilog2(bitsize);

		Buffer buffer() const noexcept
		{
			return Buffer(_data(), (_size()+7)/8);
		}
		bool isAllDisabled() const noexcept
		{
			return mem32::filled(_data(), 0, (_size() + bitsize - 1) >> bitshift);
		}
		void enable(size_t n) noexcept
		{
			_assert(n < _size());
			_data()[n >> bitshift] |= ((size_t)1 << (n & bitmask));
		}
		void disable(size_t n) noexcept
		{
			_assert(n < _size());
			_data()[n >> bitshift] &= ~((size_t)1 << (n & bitmask));
		}
		void toggle(size_t n) noexcept
		{
			_assert(n < _size());
			_data()[n >> bitshift] ^= ((size_t)1 << (n & bitmask));
		}
		void enableRange(size_t n, size_t len) noexcept
		{
			uintptr_t* p = _data() + (n >> bitshift);
			uintptr_t left = n & bitmask;
			if (left != 0)
			{
				len += left - bitsize;
				uintptr_t v = 0;
				while (left < bitsize)
				{
					v |= ((uintptr_t)1 << left);
					left++;
				}
				*p |= v;
				p++;
			}

			uintptr_t* end = p + (len >> bitshift);
			memset(p, 0xff, (byte*)end - (byte*)p);

			left = len & bitmask;
			if (left != 0)
			{
				uintptr_t v = 0;
				for (size_t i = 0; i<left; i++)
					v |= ((uintptr_t)1 << i);
				*end |= v;
			}
		}
		void disableRange(size_t n, size_t len) noexcept
		{
			uintptr_t* p = _data() + (n >> bitshift);
			uintptr_t left = n & bitmask;
			if (left != 0)
			{
				len += left - bitsize;
				uintptr_t v = 0;
				while (left < bitsize)
				{
					v |= (1 << left);
					left++;
				}
				*p &= ~v;
				p++;
			}

			uintptr_t* end = p + (len >> bitshift);
			memset(p, 0, (byte*)end - (byte*)p);

			left = len & bitmask;
			if (left != 0)
			{
				uintptr_t v = 0;
				for (size_t i = 0; i<left; i++)
					v |= (1 << i);
				*end &= ~v;
			}
		}
		void enableAll() noexcept
		{
			mem::set(_data(), 0xff, _sizep() * sizeof(uintptr_t));
		}
		void disableAll() noexcept
		{
			mem::zero(_data(), _sizep() * sizeof(uintptr_t));
		}
		void set(size_t n, bool v) noexcept
		{
			_assert(n < _size());
			uintptr_t* pmap = (_data() + (n >> 5));
			n = v << (n & bitmask);
			*pmap = (*pmap & ~n) | n;
		}
		bool get(size_t n) const noexcept
		{
			_assert(n < _size());
			return ((_data()[n >> bitshift]) & ((size_t)1 << (n & bitmask))) != 0;
		}
		size_t findEnabled(size_t index) const noexcept
		{
			const uintptr_t * data = _data();
			const uintptr_t * end = data + _sizep();
			const uintptr_t * p = data + (index >> bitshift);
			if (p >= end) return -1;

			uint left = index  & bitmask;
			if (left > 0)
			{
				uintptr_t v = *p;
				while (left < bitsize)
				{
					if (v & ((uintptr_t)1 << left))
						return index;
					left++;
					index++;
				}
				p++;
			}

			for (; p < end; p++)
			{
				if (*p == 0) continue;

				left = 0;
				uintptr_t v = *p;
				while (left < bitsize)
				{
					if (v & ((uintptr_t)1 << left))
						return (p - data)*bitsize + left;
					left++;
				}
			}
			return -1;
		}
		const bool operator [](size_t n) const noexcept
		{
			return get(n);
		}
	};
	
	template <size_t SIZE>
	class BBinArray : public BinArrayMethod<BBinArray<SIZE> >
	{
		using BinArrayMethod<BBinArray<SIZE> >::bitsize;
		static constexpr size_t _sizep = (SIZE + bitsize - 1) / bitsize;
	public:
		uintptr_t * data() noexcept
		{
			return m_map;
		}
		const uintptr_t * data() const noexcept
		{
			return m_map;
		}
		size_t size() const noexcept
		{
			return SIZE;
		}
		size_t sizep() const noexcept
		{
			return _sizep;
		}

	private:
		uintptr_t m_map[_sizep];
	};

	class BinArray : public BinArrayMethod<BinArray>
	{
	public:
		BinArray() noexcept;
		BinArray(size_t size) noexcept;
		~BinArray() noexcept;
		void alloc(size_t size) noexcept;
		uintptr_t * data() noexcept;
		const uintptr_t * data() const noexcept;
		size_t size() const noexcept;
		size_t sizep() const noexcept;
		void resizeAsFalse(size_t to) noexcept;
		void free() noexcept;

	private:
		static constexpr size_t bitsize = sizeof(uintptr_t) * 8;
		uintptr_t* m_map;

	};
}