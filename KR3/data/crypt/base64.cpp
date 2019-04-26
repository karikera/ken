#include "stdafx.h"
#include "base64.h"
#include <assert.h>

inline int decode_one(char n)
{
	if (n == '=') return -1;
	if (n == '/') return 63;
	if (n == '+') return 62;
	if (n <= '9') return (int)n - '0' + 52;
	if (n <= 'Z') return (int)n - 'A';
	if (n <= 'z') return (int)n - 'a' + 26;
	return 0;
}
inline size_t decode_24(uint8_t * dest, const char * src, size_t destlen)
{
	int n1, n2, n3, n4;
	n1 = decode_one(src[0]);
	n2 = decode_one(src[1]);
	dest[0] = ((n1 << 2) | (n2 >> 4));

	if (destlen > 1)
	{
		n3 = decode_one(src[2]);
		if (n3 == -1) return 1;
		dest[1] = ((n2 << 4) | (n3 >> 2));
	}
	if (destlen > 2)
	{
		n4 = decode_one(src[3]);
		if (n3 == -1) return 2;
		dest[2] = ((n3 << 6) | n4);
	}
	return 3;
}
inline size_t decode_24(uint8_t * dest, const char * src)
{
	int n1, n2, n3, n4;
	n1 = decode_one(src[0]);
	n2 = decode_one(src[1]);
	dest[0] = ((n1 << 2) | (n2 >> 4));

	n3 = decode_one(src[2]);
	if (n3 == -1) return 1;
	dest[1] = ((n2 << 4) | (n3 >> 2));

	n4 = decode_one(src[3]);
	if (n3 == -1) return 2;
	dest[2] = ((n3 << 6) | n4);

	return 3;
}
inline char encode_one(int n)
{
	if (n < 26) return n + 'A';
	if (n < 52) return n + (int)'a' - 26;
	if (n < 62) return n + (int)'0' - 52;
	if (n == 62) return '+';
	if (n == 63) return '/';
	return '=';
}
inline void encode_24(char * dest, const uint8_t * src, size_t srclen)
{
	dest[0] = encode_one(src[0] >> 2);
	if (srclen < 2)
	{
		dest[1] = encode_one((src[0] << 4) & 0x3f);
		dest[2] = dest[3] = '=';
		return;
	}
	dest[1] = encode_one(((src[0] << 4) | (src[1] >> 4)) & 0x3f);
	if (srclen < 3)
	{
		dest[2] = encode_one((src[1] << 2) & 0x3f);
		dest[3] = '=';
		return;
	}
	dest[2] = encode_one(((src[1] << 2) | (src[2] >> 6)) & 0x3f);
	dest[3] = encode_one(src[2] & 0x3f);
}
size_t encode64(char * dest, size_t limit, const void * psrc, size_t srclen)
{
	const uint8_t * src = (const uint8_t*)psrc;
	const uint8_t * srcend = src + srclen;
	char * deststart = dest;
	char * destend = dest + limit - 3;
	for (; src < srcend && dest < destend; src += 3, dest += 4)
	{
		encode_24(dest, src, srcend - src);
	}
	return dest - deststart;
}
size_t decode64(void * pdest, size_t limit, const char * src, size_t srclen)
{
	uint8_t * dest = (uint8_t*)pdest;
	uint8_t * destend = dest + limit;
	uint8_t * deststart = dest;

	const char * srcend = src + srclen - 3;
	for (; src < srcend && dest < destend; src += 4)
	{
		dest += decode_24(dest, src, destend - dest);
	}
	return dest - deststart;
}
size_t encode64(char * dest, const void * psrc, size_t srclen)
{
	const uint8_t * src = (const uint8_t*)psrc;
	const uint8_t * srcend = src + srclen;
	char * deststart = dest;
	for (; src < srcend; src += 3, dest += 4)
	{
		encode_24(dest, src, srcend - src);
	}
	return dest - deststart;
}
size_t decode64(void * pdest, const char * src, size_t srclen)
{
	uint8_t * dest = (uint8_t*)pdest;
	uint8_t * deststart = dest;

	const char * srcend = src + srclen - 3;
	for (; src < srcend; src += 4)
	{
		dest += decode_24(dest, src);
	}
	return dest - deststart;
}

size_t decode64_len(const char * src, size_t srclen)
{
	_assert((srclen % 4) == 0);

	srclen -= 4;
	size_t dstlen = srclen * 3;
	const char * last = src + srclen;
	if (last[2] == '=') return dstlen + 1;
	if (last[3] == '=') return dstlen + 2;
	return dstlen + 3;
}