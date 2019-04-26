#pragma once

#include <KR3/main.h>
#include "../meta/log2.h"

constexpr int MOD_ADLER = 65521;

//#pragma optimize("ts",on)

template <size_t BASE>
inline void kr::memt<BASE>::zero(ptr _dst, size_t _len) noexcept
{
	type * dst = (type*)_dst;
	type * end = dst + _len;
	while (dst != end) *dst++ = 0;
}
template <>
inline void kr::memt<1>::zero(ptr _dst, size_t _len) noexcept
{
	memset(_dst, 0, _len);
}
template <size_t BASE>
inline void kr::memt<BASE>::set(ptr _dst, atype _chr, size_t _len) noexcept
{
	type * dst = (type*)_dst;
	type * end = dst + _len;
	type cv = _chr;
	while (dst != end) *dst++ = cv;
}
template <>
inline void kr::memt<1>::set(ptr _dst, atype _chr, size_t _len) noexcept
{
	memset(_dst, (type)_chr, _len);
}
template <size_t BASE>
inline void kr::memt<BASE>::copy(ptr _dst, cptr _src, size_t _len) noexcept
{
	memcpy(_dst, _src, _len * BASE);
}
template <size_t BASE>
inline void kr::memt< BASE>::move(ptr _dst, cptr _src, size_t _len) noexcept
{
	memmove(_dst, _src, _len * BASE);
}
template <size_t BASE>
inline void kr::memt<BASE>::xor_copy(ptr _dst, cptr _src, size_t _len, dword _key) noexcept
{
	union
	{
		uintptr_t * word_dest;
		type * byte_dest;
	};
	union
	{
		uintptr_t * word_end;
		type * byte_end;
	};
	union
	{
		uintptr_t * word_src;
		type * byte_src;
	};

	uintptr_t aligncheck = (((uintptr_t)_dst) & (sizeof(uintptr_t) - 1)) | (((uintptr_t)_src) & (sizeof(uintptr_t) - 1));

	constexpr size_t SHIFT = sizeof(type) * 8;

	uintptr_t keyv = _key;
	if (sizeof(uintptr_t) > sizeof(dword))
	{
#pragma warning(push)
#pragma warning(disable:4293)
		keyv |= (keyv << (sizeof(dword) * 8));
#pragma warning(pop)
	}

	byte_dest = (type*)_dst;
	byte_src = (type*)_src;
	type * byte_real_end = (type*)_dst + _len;

	if (!(aligncheck & (sizeof(uintptr_t) - 1)))
	{
		constexpr size_t MASK = (1 << sizeof(uintptr_t)) - 1;

		byte_end = (type*)((((size_t)_dst - 1) | MASK) + 1);
		if (byte_end + sizeof(uintptr_t) <= byte_real_end)
		{
			for (; byte_dest != byte_end; byte_dest++, byte_src++)
			{
				*byte_dest = *byte_src ^ (type)_key;
				_key = intrinsic<4>::rotr(_key, SHIFT);
			}

			word_end = (uintptr_t*)(((size_t)byte_real_end) & ((size_t)-1 ^ MASK));
			do
			{
				*word_dest = *word_src ^ _key;
				word_dest++;
				word_src++;
			} while (word_dest != word_end);
		}
	}

	byte_end = byte_real_end;
	for (; byte_dest != byte_end; byte_dest++, byte_src++)
	{
		*byte_dest = *byte_src ^ (byte)_key;
		_key = intrinsic<4>::rotr(_key, SHIFT);
	}
}

template <size_t BASE>
inline kr::ptr kr::memt<BASE>::alloc(cptr _src, size_t _len) noexcept
{
	ptr _dst = _new byte[_len];
	copy(_dst, _src, _len);
	return _dst;
}

template<size_t BASE>
inline bool kr::memt<BASE>::equals(cptr _dst, cptr _src, size_t _len) noexcept
{
	return memcmp(_dst, _src, _len*BASE) == 0;
}
template<size_t BASE>
inline bool kr::memt<BASE>::equals_i(cptr _dst, cptr _src, size_t _len) noexcept
{
#ifdef _MSC_VER
	return _memicmp(_dst, _src, _len*BASE) == 0;
#else
	byte * dst = (byte*)_dst;
	byte * src = (byte*)_src;
	byte * dstend = dst + _len;

	while (dst != dstend)
	{
		byte src_chr = *src++;
		byte dst_chr = *dst++;
		if (dst_chr == src_chr)
			continue;
		if (src_chr < 'A')
			return false;
		if (src_chr <= 'Z')
		{
			// A~Z
			if ((byte)(src_chr + ('a'- 'A')) == dst_chr)
				continue;
			return false;
		}
		if (src_chr < 'a')
			return false;
		if (src_chr > 'z')
			return false;
		// a~z
		if ((byte)(src_chr + ('A' - 'a')) == dst_chr)
			continue;
		return false;
	}
	return true;
#endif
}
template<size_t BASE>
inline int kr::memt<BASE>::compare(cptr _dst, cptr _src, size_t _len) noexcept
{
	union
	{
		uintptr_t * word_dest;
		type * byte_dest;
	};
	union
	{
		uintptr_t * word_end;
		type * byte_end;
	};
	union
	{
		uintptr_t * word_src;
		type * byte_src;
	};

	uintptr_t aligncheck = (((uintptr_t)_dst) & (sizeof(uintptr_t) - 1)) | (((uintptr_t)_src) & (sizeof(uintptr_t) - 1));

#pragma warning(push)
#pragma warning(disable:4307)
	static constexpr size_t WORDSIZE = sizeof(uintptr_t) / BASE;
#pragma warning(pop)
	
	constexpr size_t SHIFT = sizeof(type) * 8;

	byte_dest = (type*)_dst;
	byte_src = (type*)_src;
	type * byte_real_end = (type*)_dst + _len;
	
	if (!(aligncheck & (sizeof(uintptr_t) - 1)))
	{
		constexpr size_t MASK = (1 << sizeof(uintptr_t)) - 1;
		byte_end = (type*)((((size_t)_dst - 1) | MASK) + 1);
		if (byte_end + sizeof(uintptr_t) <= byte_real_end)
		{
			for (; byte_dest != byte_end; byte_dest++, byte_src++)
			{
				if (*byte_dest != *byte_src)
				{
					return (*byte_dest < *byte_src) ? -1 : 1;
				}
			}

			word_end = (uintptr_t*)(((size_t)byte_real_end) & ((size_t)-1 ^ MASK));
			do
			{
				if (*word_dest != *word_src)
				{
					byte_end = byte_dest + WORDSIZE;
					goto __left;
				}
				word_src++;
				word_dest++;
			} while (word_dest != word_end);
		}
	}

	byte_end = byte_real_end;
__left:
	for (;byte_dest != byte_end; byte_dest++, byte_src++)
	{
		if (*byte_dest != *byte_src)
		{
			return (*byte_dest < *byte_src) ? -1 : 1;
		}
	}
	return 0;
}
template <>
inline ATTR_CHECK_RETURN int kr::memt<1>::compare(cptr _dst, cptr _src, size_t _len) noexcept
{
	return memcmp(_dst, _src, _len);
}
template<size_t BASE>
template <typename T>
inline T * kr::memt<BASE>::compare_p(T * _dst, tconst<T>* _src, size_t _len) noexcept
{
	union
	{
		uintptr_t * word_dest;
		type * byte_dest;
	};
	union
	{
		uintptr_t * word_end;
		type * byte_end;
	};
	union
	{
		uintptr_t * word_src;
		type * byte_src;
	};

	uintptr_t aligncheck = (((uintptr_t)_dst) & (sizeof(uintptr_t) - 1)) | (((uintptr_t)_src) & (sizeof(uintptr_t) - 1));

#pragma warning(push)
#pragma warning(disable:4307)
	static constexpr size_t WORDSIZE = sizeof(uintptr_t) / BASE;
#pragma warning(pop)
	
	constexpr size_t SHIFT = sizeof(type) * 8;

	byte_dest = (type*)_dst;
	byte_src = (type*)_src;
	type * byte_real_end = (type*)_dst + _len;
	
	if (!(aligncheck & (sizeof(uintptr_t) - 1)))
	{
		constexpr size_t MASK = (1 << sizeof(uintptr_t)) - 1;
		byte_end = (type*)((((size_t)_dst - 1) | MASK) + 1);
		if (byte_end + sizeof(uintptr_t) <= byte_real_end)
		{
			for (; byte_dest != byte_end; byte_dest++, byte_src++)
			{
				if (*byte_dest != *byte_src)
				{
					return byte_dest;
				}
			}

			word_end = (uintptr_t*)(((size_t)byte_real_end) & ((size_t)-1 ^ MASK));
			do
			{
				if (*word_dest != *word_src)
				{
					byte_end = byte_dest + WORDSIZE;
					goto __left;
				}
				word_src++;
				word_dest++;
			} while (word_dest != word_end);
		}
	}
	byte_end = byte_real_end;

__left:
	for (;byte_dest != byte_end; byte_dest++, byte_src++)
	{
		if (*byte_dest != *byte_src)
		{
			return byte_dest;
		}
	}
	return 0;
}

template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::strlen(T* _dst) noexcept
{
	return find(_dst, (T)0) - _dst;
}
template <>
template <typename T>
inline size_t kr::memt<1>::strlen(T* _dst) noexcept
{
	return ::strlen((char*)_dst);
}
template <size_t BASE>
template <typename T, typename READCB>
inline auto kr::memt<BASE>::find_callback(const READCB &read, T* _tar, size_t _tarlen)->decltype(read())
{
	_assert(_tarlen != 0);
	size_t tarhash = 0;
	size_t srchash = 0;

	{
		auto * t = _tar;
		auto * te = t + _tarlen;
		while (t != te)
		{
			tarhash ^= *t;
			tarhash = intrinsic<sizeof(size_t)>::rotl(tarhash, 9);
			t++;
		}
	}

	using unconstT = std::remove_const_t<T>;
	Array<unconstT> tmp(_tarlen);
	unconstT * tmpbeg = tmp.data();
	unconstT * tmpend = tmpbeg + _tarlen;
	unconstT * tmpiter = tmpbeg;
	decltype(read()) srcptr;
	{
		do
		{
			srcptr = read();
			*tmpiter++ = *srcptr;
			srchash ^= *srcptr;
			srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
		}
		while (tmpiter != tmpend);
	}
	tmpiter = tmpbeg;

	int hashoff = (9 * _tarlen) & (sizeof(size_t) * 8 - 1);
	{
		size_t hashcap = sizeof(size_t) * 32 / 8;
		if (_tarlen < hashcap) _tarlen = 0;
		else _tarlen -= hashcap;
	}
	for (;;)
	{
		if (tarhash == srchash)
		{
			size_t left_part_sz = tmpend - tmpiter;
			if (left_part_sz >= _tarlen)
			{
				if (equals(tmpiter, _tar, _tarlen)) break;
			}
			else
			{
				if (equals(tmpiter, _tar, left_part_sz))
				{
					T * right_part = _tar + left_part_sz;
					size_t right_part_sz = _tarlen - left_part_sz;
					if (equals(tmpbeg, right_part, right_part_sz)) break;
				}
			}
		}
		srchash ^= intrinsic<sizeof(size_t)>::rotl(*tmpiter, hashoff);
		srcptr = read();
		*tmpiter++ = *srcptr;
		if (tmpiter == tmpend) tmpiter = tmpbeg;
		srchash ^= *srcptr;
		srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
	}

	return srcptr+1;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find(T* _dst, atype _tar) noexcept
{
	type* dst = (type*)_dst;
	type tar = _tar;
	while (*dst != tar) dst++;
	return (T*)dst;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find(T* _dst, atype _tar, size_t _dstsize) noexcept
{
	type tar = _tar;
	type* dst = (type*)_dst;
	type* dstend = dst + _dstsize;
	while (dst != dstend)
	{
		if (*dst == tar) return (T*)dst;
		dst++;
	}
	return nullptr;
}
template <>
template <typename T>
inline T* kr::memt<1>::find(T* _dst, atype _tar, size_t _dstsize) noexcept
{
	return (T*)memchr(_dst, (type)_tar, _dstsize);
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_e(T* _src, atype _tar, size_t _srclen) noexcept
{
	type tar = _tar;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		if (*src == tar) break;
		src++;
	};
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept
{
	_assert(_tarlen != 0);
	size_t tarhash = 0;
	size_t srchash = 0;

	{
		auto * t = _tar;
		auto * te = t + _tarlen;
		while (t != te)
		{
			tarhash ^= *t;
			tarhash = intrinsic<sizeof(size_t)>::rotl(tarhash, 9);
			t++;
		}
	}
	T * src_right = _src;
	{
		T * end = src_right + _tarlen;
		while (src_right != end)
		{
			srchash ^= *src_right;
			srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
			src_right++;
		}
	}

	int hashoff = (9 * _tarlen) & (sizeof(size_t) * 8 - 1);
	{
		size_t hashcap = sizeof(size_t) * 32 / 8;
		if (_tarlen < hashcap) _tarlen = 0;
		else _tarlen -= hashcap;
	}

	for (;;)
	{
		if (tarhash == srchash && equals(_src, _tar, _tarlen))
		{
			return _src;
		}
		srchash ^= intrinsic<sizeof(size_t)>::rotl(*_src, hashoff);
		srchash ^= *src_right;
		srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
		_src++;
		src_right++;
	}
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	_assert(_tarlen != 0);
	if (_srclen < _tarlen) return nullptr;
	size_t tarhash = 0;
	size_t srchash = 0;

	{
		auto * t = _tar;
		auto * te = t + _tarlen;
		while (t != te)
		{
			tarhash ^= *t;
			tarhash = intrinsic<sizeof(size_t)>::rotl(tarhash, 9);
			t++;
		}
	}
	T * _src2 = _src;
	{
		T * end = _src2 + _tarlen;
		while (_src2 != end)
		{
			srchash ^= *_src2;
			srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
			_src2++;
		}
	}

	T * srcend = _src + _srclen;
	int hashoff = (9 * _tarlen) & (sizeof(size_t) * 8 - 1);
	{
		size_t hashcap = sizeof(size_t) * 32 / 8;
		if (_tarlen < hashcap) _tarlen = 0;
		else _tarlen -= hashcap;
	}

	for (;;)
	{
		if (tarhash == srchash && equals(_src, _tar, _tarlen))
			return _src;
		if (_src2 == srcend)
			return nullptr;
		srchash ^= intrinsic<sizeof(size_t)>::rotl(*_src, hashoff);
		srchash ^= *_src2;
		srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
		_src++;
		_src2++;
	}
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_r(T* _src, atype _tar, size_t _len) noexcept
{
	type tar = _tar;
	type* srcend = (type*)_src - 1;
	type* src = srcend + _len;
	while (src != srcend)
	{
		if (*src == tar) return (T*)src;
		src--;
	};
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_r(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	_assert(_tarlen != 0);
	if (_srclen < _tarlen)
		return nullptr;
	size_t tarhash = 0;
	size_t srchash = 0;

	{
		auto * t = _tar + _tarlen;
		auto * te = _tar;
		while (t != te)
		{
			t--;
			tarhash ^= *t;
			tarhash = intrinsic<sizeof(size_t)>::rotl(tarhash, 9);
		}
	}
	T * p = _src + _srclen;
	T * _src2 = p;
	{
		T * end = p - _tarlen;
		while (_src2 != end)
		{
			_src2--;
			srchash ^= *_src2;
			srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
		}
	}

	T * srcend = _src - 1;
	int hashoff = (9 * _tarlen) & (sizeof(size_t) * 8 - 1);
	_tarlen--;

	for (;;)
	{
		if (tarhash == srchash && equals(_src2, _tar, _tarlen))
			return _src2;
		if (_src2 == srcend)
			return nullptr;

		_src2--;
		p--;
		srchash ^= intrinsic<sizeof(size_t)>::rotl(*p, hashoff);
		srchash ^= *_src2;
		srchash = intrinsic<sizeof(size_t)>::rotl(srchash, 9);
	}
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_ry(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type* srcend = (type*)_src - 1;
	type* src = srcend + _srclen;

	while (src != srcend)
	{
		if(contains(_tar, *src, _tarlen)) return (T*)src;
		src--;
	};
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_re(T* _src, atype _tar, size_t _len) noexcept
{
	type tar = _tar;
	type* srcend = (type*)_src - 1;
	type* src = srcend + _len;
	while (src != srcend)
	{
		if (*src == tar) break;
		src--;
	};
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_rye(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type tar = _tar;
	type* srcend = (type*)_src - 1;
	type* src = srcend + _srclen;
	while (src != srcend)
	{
		if (contains(_tar, *src, _tarlen)) break;
		src--;
	};
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_y(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept
{
	type * src = (type*)_src;

	for (;;)
	{
		if (contains(_tar, *src, _tarlen)) return (T*)src;
		src++;
	}
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_y(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type * src = (type*)_src;
	type * srcend = (type*)_src + _srclen;

	while (src != srcend)
	{
		if (contains(_tar, *src, _tarlen)) return (T*)src;
		src++;
	}
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_ye(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type * src = (type*)_src;
	type * srcend = (type*)_src + _srclen;

	while (src != srcend)
	{
		if (contains(_tar, *src, _tarlen)) break;
		src++;
	}
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_n(T* _src, atype _skp) noexcept
{
	type tar = _skp;
	type* src = (type*)_src;
	while (*src == tar) src++;
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_n(T* _src, atype _skp, size_t _srclen) noexcept
{
	type tar = _skp;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		if (*src != tar) return (T*)src;
		src++;
	}
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_nr(T* _src, atype _tar, size_t _len) noexcept
{
	type tar = _tar;
	type* srcend = (type*)_src - 1;
	type* src = srcend + _len;
	while (src != srcend)
	{
		if (*src != tar) return (T*)src;
		src--;
	};
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_ne(T* _src, atype _skp, size_t _srclen) noexcept
{
	type tar = _skp;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		if (*src != tar) return (T*)src;
		src++;
	}
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_ny(T* _src, tconst<T>* _skp, size_t _skplen) noexcept
{
	type * src = (type*)_src;

	for (;;)
	{
		if (!contains(_skp, *src, _skplen)) return (T*)src;
		src++;
	}
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_ny(T* _src, tconst<T>* _skp, size_t _srclen, size_t _skplen) noexcept
{
	type * src = (type*)_src;
	type * srcend = (type*)_src + _srclen;

	while (src != srcend)
	{
		if (!contains(_skp, *src, _skplen)) return (T*)src;
		src++;
	}
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_nre(T* _src, atype _tar, size_t _len) noexcept
{
	type tar = _tar;
	type* srcend = (type*)_src - 1;
	type* src = srcend + _len;
	while (src != srcend)
	{
		if (*src != tar) break;
		src--;
	};
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_nry(T* _src, tconst<T>* _skp, size_t _srclen, size_t _skplen) noexcept
{
	type* srcend = (type*)_src - 1;
	type* src = srcend + _srclen;

	while (src != srcend)
	{
		if (!contains(_skp, *src, _skplen)) return (T*)src;
		src--;
	};
	return nullptr;
}
template <size_t BASE>
template <typename T>
inline T* kr::memt<BASE>::find_nye(T* _src, tconst<T>* _skp, size_t _srclen, size_t _skplen) noexcept
{
	type * src = (type*)_src;
	type * srcend = (type*)_src + _srclen;

	while (src != srcend)
	{
		if (!contains(_skp, *src, _skplen)) break;
		src++;
	}
	return (T*)src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos(T* _src, atype _tar) noexcept
{
	type* str = (type*)find(_src, _tar);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos(T* _src, atype _tar, size_t _srclen) noexcept
{
	type* str = (type*)find(_src, _tar, _srclen);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept
{
	return (type*)find(_src, _tar, _tarlen) - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type* str = (type*)find(_src, _tar, _srclen, _tarlen);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_r(T* _src, atype _tar, size_t _srclen) noexcept
{
	return (type*)find_re(_src, _tar, _srclen) - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_ry(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type* str = (type*)find_ry(_src, _tar, _srclen, _tarlen);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_y(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept
{
	return (type*)find_y(_src, _tar, _tarlen) - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_y(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type* str = (type*)find_y(_src, _tar, _srclen, _tarlen);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_n(T* _src, atype _tar) noexcept
{
	type* str = (type*)find_n(_src, _tar);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_n(T* _src, atype _tar, size_t _srclen) noexcept
{
	type* str = (type*)find_n(_src, _tar, _srclen);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_nr(T* _src, atype _tar, size_t _srclen) noexcept
{
	return (type*)find_nre(_src, _tar, _srclen) - (type*)_src;
}
template <size_t BASE>
template <typename T>
inline size_t kr::memt<BASE>::pos_nry(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept
{
	type* str = (type*)find_nry(_src, _tar, _srclen, _tarlen);
	if (str == nullptr) return -1;
	return str - (type*)_src;
}

template <size_t BASE>
inline bool kr::memt<BASE>::contains(cptr _src, atype _tar, size_t _srclen) noexcept
{
	type tv = _tar;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		if (*src == tv) return true;
		src++;
	}
	return false;
}
template <size_t BASE>
inline bool kr::memt<BASE>::filled(cptr _src, atype _tar, size_t _srclen) noexcept
{
	type tv = _tar;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		if (*src != tv) return false;
		src++;
	}
	return true;
}
template<size_t BASE>
inline size_t kr::memt<BASE>::count(cptr _src, atype _tar, size_t _srclen) noexcept
{
	type tv = _tar;
	size_t count = 0;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		if (*src == tv) count++;
		src++;
	}
	return count;
}
template<size_t BASE>
inline size_t kr::memt<BASE>::count_y(cptr _src, cptr _tar, size_t _srclen, size_t _tarlen) noexcept
{
	size_t count = 0;
	type* tar = (type*)_tar;
	type* tarend = tar + _tarlen;
	type* src = (type*)_src;
	type* srcend = src + _srclen;
	while (src != srcend)
	{
		type s = *src;
		type * t = tar;
		while (t != tarend)
		{
			if (s == *t)
			{
				count++;
				break;
			}
			t++;
		}
		src++;
	}
	return count;
}

template <size_t BASE>
inline void kr::memt<BASE>::change(ptr _src, atype _tar, atype _to, size_t _srclen) noexcept
{
	type* src = (type*)_src;
	type* end = src + _srclen;
	type tv = _tar;
	type tov = _to;
	while (src != end)
	{
		if (*src == tv) *src = tov;
		src++;
	}
}
template <size_t BASE>
inline void kr::memt<BASE>::tolower(ptr _src, size_t _srclen) noexcept
{
	type* src = (type*)_src;
	type* end = src + _srclen;
	while (src != end)
	{
		type &v = *src;
		if ((type)'A' <= v && v <= (type)'Z')
			v += (type)'a' - (type)'A';
		src++;
	}
}
template <size_t BASE>
inline void kr::memt<BASE>::toupper(ptr _src, size_t _srclen) noexcept
{
	type* src = (type*)_src;
	type* end = src + _srclen;
	while (src != end)
	{
		type &v = *src;
		if ((type)'a' <= v && v <= (type)'z')
			v -= (type)'a' - (type)'A';
		src++;
	}
}
template <size_t BASE>
inline size_t kr::memt<BASE>::hash(cptr _src, size_t _len) noexcept
{
	size_t * ptr = (size_t*)_src;
	_len *= BASE;
	size_t * end = ptr + _len / sizeof(size_t);

	size_t a = 1;
	size_t b = 0;

	for (; ptr != end; ptr++)
	{
		a = (a + *ptr) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}

	size_t tail = 0;
	memcpy(&tail, ptr, _len% sizeof(size_t));
	a = (a + tail) % MOD_ADLER;
	b = (b + a) % MOD_ADLER;

	return (b << 16) | a;
}
template<size_t BASE>
inline bool kr::memt<BASE>::numberonly(cptr _src, size_t _len) noexcept
{
	type* src = (type*)_src;
	type* srcend = src + _len;
	while (src != srcend)
	{
		type chr = *src;
		if ('0' > chr || chr > '9') return false;
		src++;
	}
	return true;
}
template<size_t BASE>
template<typename T>
inline T kr::memt<BASE>::toint(cptr _src, size_t _len, uint _radix) noexcept
{
	return _pri_::AutoFunc<BASE, std::is_signed<T>::value>::template toint<T>(_src, _len, _radix);
}
template<size_t BASE>
template <typename T>
inline T kr::memt<BASE>::toint_limit(cptr _src, size_t _len, uint _radix) throws(OutOfRangeException)
{
	return _pri_::AutoFunc<BASE, std::is_signed<T>::value>::template toint_limit<T>(_src, _len, _radix);
}
template<size_t BASE>
template<typename T>
inline T kr::memt<BASE>::touint(cptr _src, size_t _len, uint _radix) noexcept
{
	static_assert(std::is_unsigned<T>::value, "Must be unsigned number");
	type* src = (type*)_src;
	type* srcend = src + _len;
	T out = 0;
	goto __start;

__skip:
	src++;
__start:
	if (src != srcend)
	{
		for (;;)
		{
			type chr = *src;
			if (chr < '0') goto __skip;
			else if (chr <= '9') out += (T)(chr - '0');
			else if (chr < 'A') goto __skip;
			else if (chr <= 'Z') out += (T)(chr - ('A' - 10));
			else if (chr < 'a') goto __skip;
			else if (chr <= 'z') out += (T)(chr - ('a' - 10));
			else goto __skip;
			src++;
			if (src == srcend) break;
			out *= _radix;
		}
	}
	return out;
}
template<size_t BASE>
template <typename T>
inline T kr::memt<BASE>::touint_limit(cptr _src, size_t _len, uint _radix) throws(OutOfRangeException)
{
	static_assert(std::is_unsigned<T>::value, "Must be unsigned number");
	type* src = (type*)_src;
	type* srcend = src + _len;
	T out = 0;
	goto __start;

__skip:
	src++;
__start:
	if (src != srcend)
	{
		for (;;)
		{
			type chr = *src;

			T n;
			if (chr < '0') goto __skip;
			else if (chr <= '9') n = (T)(chr - '0');
			else if (chr < 'A') goto __skip;
			else if (chr <= 'Z') n = (T)(chr - ('A' - 10));
			else if (chr < 'a') goto __skip;
			else if (chr <= 'z') n = (T)(chr - ('a' - 10));
			else goto __skip;
			n += out;
			if (out > n) throw OutOfRangeException();
			out = n;
			src++;

			if (src == srcend) break;
			out *= _radix;
		}
	}
	return out;
}
template<size_t BASE>
template<typename T>
inline T kr::memt<BASE>::tosint(cptr _src, size_t _len, uint _radix) noexcept
{
	static_assert(std::is_signed<T>::value, "Must be signed number");
	using UT = std::make_unsigned_t<T>;
	if (_len == 0) return 0;
	if (*(type*)_src == (type)'-') return -(T)touint<UT>((type*)_src+1, _len-1, _radix);
	else return (T)touint<UT>(_src, _len, _radix);
}
template<size_t BASE>
template <typename T>
inline T kr::memt<BASE>::tosint_limit(cptr _src, size_t _len, uint _radix) throws(OutOfRangeException)
{
	static_assert(std::is_signed<T>::value, "Must be signed number");
	using UT = std::make_unsigned_t<T>;

	if (_len == 0) return 0;
	if (*(type*)_src == (type)'-')
	{
		UT v = touint<UT>((type*)_src + 1, _len - 1, _radix);
		if (v > (UT)minof(T)) throw OutOfRangeException();
		return -(T)v;
	}
	else
	{
		qword v = (T)touint<UT>((type*)_src, _len, _radix);
		if (v > (UT)maxof(T)) throw OutOfRangeException();
		return (T)v;
	}
}
template<size_t BASE>
template<typename T>
inline void kr::memt<BASE>::fromint(ptr _dst, size_t _cipher, T _number, uint _radix) noexcept
{
	std::make_unsigned_t<T> val = _number;

	type* p = (type*)_dst + _cipher - 1;
	for (;;)
	{
		T res = val % _radix;
		val /= _radix;
		*p = inttochr(res);
		if (_dst == p) break;
		p--;
	}
}

template <size_t SIZE>
inline void kr::_pri_::SIZE_MEM<SIZE>::fill(void* dest, const void* src, size_t size) noexcept
{
	using comptype = typename almem::type;
	if (align == SIZE) almem::set(dest, *(comptype*)src, size);
	else
	{
		comptype* dst = (comptype*)dest;
		comptype* end = dst + size;
		while (dst != end)
		{
			*dst = *(comptype*)src;
			dst++;
		}
	}
}
template <size_t SIZE>
inline void kr::_pri_::SIZE_MEM<SIZE>::copy(void* dest, const void* src, size_t size) noexcept
{
	almem::copy(dest, src, size*blocksize);
}
template <size_t SIZE>
inline void kr::_pri_::SIZE_MEM<SIZE>::move(void* dest, const void* src, size_t size) noexcept
{
	almem::move(dest, src, size*blocksize);
}
template <size_t SIZE>
inline void kr::_pri_::SIZE_MEM<SIZE>::zero(void* dest, size_t size) noexcept
{
	almem::zero(dest, size*blocksize);
}
template <size_t SIZE>
inline void kr::_pri_::SIZE_MEM_SINGLE<SIZE>::copy(void* dest, const void* src) noexcept
{
	SIZE_MEM<SIZE>::copy(dest, src, 1);
}
template <size_t SIZE>
inline void kr::_pri_::SIZE_MEM_SINGLE<SIZE>::zero(void* dest) noexcept
{
	SIZE_MEM<SIZE>::zero(dest, 1);
}

template <typename T>
void kr::_pri_::ARRCOPY<true, true>::subs_copy(void* dest, const void* src, size_t size) noexcept
{
	memcpy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::subs_move(void* dest, void* src, size_t size) noexcept
{
	memcpy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::ctor(void * dest, size_t size) noexcept
{
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::dtor(void * dest, size_t size) noexcept
{
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::ctor_copy(void* dest, const void* src, size_t size) noexcept
{
	memcpy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::ctor_move(void* dest, void* src, size_t size) noexcept
{
	memcpy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::ctor_move_r(void* dest, void* src, size_t size) noexcept
{
	memmove(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::ctor_move_d(void* dest, void* src, size_t size) noexcept
{
	memcpy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<true, true>::ctor_move_rd(void* dest, void* src, size_t size) noexcept
{
	memmove(dest, src, size);
}

template <typename T>
void kr::_pri_::ARRCOPY<true, false>::subs_fill(T* dest, const T& src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		*dest++ = src;
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::subs_copy(T* dest, const T* src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		*dest++ = *src++;
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::subs_move(T* dest, T* src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		*dest++ = move(*src++);
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor(T * dest, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		new(dest++) T;
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::dtor(T * dest, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		(dest++)->~T();
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor_fill(T* dest, const T& src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		new(dest++) T(src);
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor_copy(T* dest, const T* src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		new(dest++) T(*src++);
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor_move(T* dest, T* src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		new(dest++)T(move(*src++));
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor_move_r(T* dest, T* src, size_t size) noexcept
{
	dest--;
	src--;
	T * end = dest;
	dest += size;
	src += size;
	while (dest != end)
	{
		new(end--)T(move(*src--));
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor_move_d(T* dest, T* src, size_t size) noexcept
{
	T * end = dest + size;
	while (dest != end)
	{
		new(dest++) T(move(*src));
		(src++)->~T();
	}
}
template <typename T>
void kr::_pri_::ARRCOPY<true, false>::ctor_move_rd(T* dest, T* src, size_t size) noexcept
{
	dest--;
	src--;
	T * end = dest;
	dest += size;
	src += size;
	while (dest != end)
	{
		new(dest--) T(move(*src));
		(src--)->~T();
	}
}

template <typename T>
void kr::_pri_::ARRCOPY<false, false>::subs_fill(T* dest, const T& src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::fill(dest, &src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::subs_copy(T* dest, const T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::copy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::subs_move(T* dest, T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::copy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor(T * dest, size_t size) noexcept
{
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::dtor(T * dest, size_t size) noexcept
{
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor_fill(T* dest, const T& src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::fill(dest, &src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor_copy(T* dest, const T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::copy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor_move(T* dest, T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::copy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor_move_r(T* dest, T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::move(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor_move_d(T* dest, T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::copy(dest, src, size);
}
template <typename T>
void kr::_pri_::ARRCOPY<false, false>::ctor_move_rd(T* dest, T* src, size_t size) noexcept
{
	SIZE_MEM<sizeof(T)>::move(dest, src, size);
}


template <typename T> T* kr::mema::find(T* _src, add_const_t<T> &_tar) noexcept
{
	for (;;)
	{
		if (*_src == _tar) return _src;
		_src++;
	}
}
template <typename T> T* kr::mema::find(T* _src, add_const_t<T> &_tar, size_t _len) noexcept
{
	T * srcend = _src + _len;
	while (_src != srcend)
	{
		if (*_src == _tar) return _src;
		_src++;
	}
	return nullptr;
}
template <typename T> size_t kr::mema::pos(const T* _src, const T &_tar) noexcept
{
	return find(_src, _tar) - _src;
}
template <typename T> size_t kr::mema::pos(const T* _src, const T &_tar, size_t _len) noexcept
{
	T* res = find(_src, _tar, _len);
	if (res == nullptr) return -1;
	return res - _src;
}
template <typename T> void kr::mema::zero(T& _dst) noexcept
{
	_pri_::SIZE_MEM_SINGLE<sizeof(T)>::zero(&_dst);
}
template <typename T> void kr::mema::zero(T* _dst, size_t _len) noexcept
{
	_pri_::SIZE_MEM<sizeof(T)>::zero(_dst, _len);
}
template <typename T> void kr::mema::copy(T& _dst, const T &_src) noexcept
{
	_pri_::SIZE_MEM_SINGLE<sizeof(T)>::copy(&_dst, &_src);
}
template <typename T> void kr::mema::copy(T* _dst, const T* _src, size_t _len) noexcept
{
	_pri_::SIZE_MEM<sizeof(T)>::copy(_dst, _src, _len);
}
template <typename T> void kr::mema::copy_r(T* _dst, const T* _src, size_t _len) noexcept
{
	_pri_::SIZE_MEM<sizeof(T)>::copy_r(_dst, _src, _len);
}
template <typename T, size_t size> void kr::mema::copy(T(&dest)[size], const T(&src)[size]) noexcept
{
	copy(dest, src, size);
}

template <typename T> T* kr::mema::alloc(const T &_src) noexcept
{
	T* _dst = (T*)_new byte[sizeof(T)];
	return new(_dst) T(_src);
}
template <typename T> T* kr::mema::alloc(const T* _src, size_t _len) noexcept
{
	T* _dst = (T*)_new byte[sizeof(T) + _len];
	ctor_copy(_dst, _src, _len);
	return _dst;
}

template <typename T> void kr::mema::ctor(T* dest, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor<T>(dest, size);
}
template <typename T> void kr::mema::dtor(T* dest, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template dtor<T>(dest, size);
}
template <typename T> void kr::mema::subs_fill(T* dest, const T& src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template subs_fill<T>(dest, src, size);
}
template <typename T> void kr::mema::subs_copy(T* dest, const T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template subs_copy<T>(dest, src, size);
}
template <typename T> void kr::mema::subs_move(T* dest, T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template subs_move<T>(dest, src, size);
}
template <typename T> void kr::mema::ctor_fill(T* dest, const T& src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor_fill<T>(dest, src, size);
}
template <typename T> void kr::mema::ctor_copy(T* dest, const T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor_copy<T>(dest, src, size);
}
template <typename T> void kr::mema::ctor_move(T* dest, T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor_move<T>(dest, src, size);
}
template <typename T> void kr::mema::ctor_move_r(T* dest, T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor_move_r<T>(dest, src, size);
}
template <typename T> void kr::mema::ctor_move_d(T* dest, T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor_move_d<T>(dest, src, size);
}
template <typename T> void kr::mema::ctor_move_rd(T* dest, T* src, size_t size) noexcept
{
	_pri_::ARRCOPY<!std::is_trivially_default_constructible<T>::value, std::is_void<T>::value>::template ctor_move_rd<T>(dest, src, size);
}
template <typename T, size_t size> void kr::mema::subs_copy(T(&dest)[size], const T(&src)[size]) noexcept
{
	subs_copy(dest, src, size);
}
template <typename T, size_t size> void kr::mema::subs_move(T(&dest)[size], const T(&src)[size]) noexcept
{
	subs_move(dest, src, size);
}

//#pragma optimize("",on)
