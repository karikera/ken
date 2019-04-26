#include "stdafx.h"
#include "crypt.h"

#include <KR3/data/map.h>

#include "crypt/base64.h"
#include "crypt/sha1.h"
#include "crypt/md5.h"
#include "crypt/sha256.h"

using namespace kr;
using namespace encoder;

namespace
{
	ReferenceMap<Text, Text> * getEntities() noexcept
	{
		static ReferenceMap<Text, Text> entities = {
			{ "gt", ">" },
			{ "lt", "<" },
			{ "quot", "\"" },
		};
		return &entities;
	}
}

size_t HtmlEntity::length(Text text) noexcept
{
	size_t length = 0;
	for (char chr : text)
	{
		switch (chr)
		{
		case '<': length += 4; break;
		case '>': length += 4; break;
		case '"': length += 6; break;
		default: length++; break;
		}
	}
	return length;
}
void HtmlEntity::encode(Writer * out, Text * text) noexcept
{
	const char * src = text->begin();
	const char * send = text->end();
	char * dest = out->end();
	char * dend = out->limit();

	auto write = [&](Text text){
		size_t sz = text.size();
		char * dest_next = dest + sz;
		if (dest_next > dend)
		{
			src--;
			return;
		}
		text.copyTo(dest);
		dest = dest_next;
	};

	while (src != send)
	{
		char chr = *src++;
		switch (chr)
		{
		case '<': write("&lt;"); break;
		case '>': write("&gt;"); break;
		case '"': write("&quot;"); break;
		default:
			if (dest == dend)
			{
				src--;
				break;
			}
			*dest++ = chr;
			break;
		}
	}

	*out = Writer(dest, dend);
	*text = Text(src, send);
}
size_t HtmlEntity::encode(char * out, Text text) noexcept
{
	char * dest = out;
	for (char chr : text)
	{
		if (chr >= 0x20 && chr != '=' && chr != '&')
		{
			*dest++ = chr;
		}
		else
		{
			*dest++ = '%';
			*dest++ = mem::inttochr(chr & 0xf);
			*dest++ = mem::inttochr(chr >> 4);
		}
	}
	return dest - out;
}
size_t HtmlEntity::delength(Text text) noexcept
{
	auto * entities = getEntities();
	size_t length = 0;
	const char * src = text.begin();
	const char * end = text.end();
	while (src != end)
	{
		char chr = *src++;
		if (chr == '&')
		{
			const char * from = src;
			Text entity;
			for (;;)
			{
				if (src == end) goto _end;
				char chr2 = *src;
				if (chr2 == ';')
				{
					entity = Text(from, src);
					src++;
					break;
				}
				else if ((char)chr2 <= 0x20)
				{
					entity = Text(from, src);
					break;
				}
				src++;
			}
			if (entity.startsWith('#'))
			{
				entity++;
				length++;
			}
			else
			{
				auto iter = entities->find(entity);
				Text replace;
				if (iter == entities->end())
				{
					replace = Text(from - 1, src);
				}
				else
				{
					replace = iter->second;
				}
				size_t sz = replace.size();
				length += sz;
			}
		}
		else
		{
			length++;
		}
	}
_end:
	return length;
}
size_t HtmlEntity::decode(char *out, Text text) noexcept
{
	auto * entities = getEntities();

	char * dest = out;
	const char * src = text.begin();
	const char * end = text.end();
	while (src != end)
	{
		char chr = *src++;
		if (chr == '&')
		{
			const char * from = src;
			Text entity;
			for (;;)
			{
				if (src == end) goto _end;
				char chr2 = *src;
				if (chr2 == ';')
				{
					entity = Text(from, src);
					src++;
					break;
				}
				else if ((char)chr2 <= 0x20)
				{
					entity = Text(from, src);
					break;
				}
				src++;
			}
			if (entity.startsWith('#'))
			{
				entity++;
				*dest++ = (char)entity.to_uint();
			}
			else
			{
				auto iter = entities->find(entity);
				Text replace;
				if (iter == entities->end())
				{
					replace = Text(from - 1, src);
				}
				else
				{
					replace = iter->second;
				}
				size_t sz = replace.size();
				memcpy(dest, replace.data(), sz);
				dest += sz;
			}
		}
		else
		{
			*dest++ = chr;
		}
	}
_end:
	return dest - out;
}
void HtmlEntity::decode(Writer *out, Text * text) noexcept
{
	auto * entities = getEntities();

	const char * src = text->begin();
	const char * end = text->end();
	char * dest = out->end();
	char * dend = out->limit();

	while (src != end)
	{
		char chr = *src++;
		if (chr == '&')
		{
			const char * from = src;
			Text entity;
			for (;;)
			{
				if (src == end) goto _end;
				char chr2 = *src++;
				if (chr2 == ';')
				{
					entity = Text(from, src);
					src++;
					break;
				}
				else if ((char)chr2 <= 0x20)
				{
					entity = Text(from, src);
					break;
				}
			}
			if (entity.startsWith('#'))
			{
				if (dest == dend)
				{
					src = from - 1;
					break;
				}
				entity++;
				*dest++ = (char)entity.to_uint();
			}
			else
			{
				auto iter = entities->find(entity);
				Text replace;
				if (iter == entities->end())
				{
					replace = Text(from - 1, src);
				}
				else
				{
					replace = iter->second;
				}
				size_t sz = replace.size();
				if (dest + sz > dend)
				{
					src = from - 1;
					break;
				}
				memcpy(dest, replace.data(), sz);
				dest += sz;
			}
		}
		else
		{
			if (dest == dend)
			{
				src--;
				break;
			}
			*dest++ = chr;
		}
	}
_end:

	out->_setEnd(dest);
	text->setBegin(src);
}

size_t Uri::length(Text text) noexcept
{
	size_t length = 0;
	for (char chr : text)
	{
		length += (chr < 0x20 || chr == '=' || chr == '&') * 2 + 1;
	}
	return length;
}
void Uri::encode(Writer * out, Text * text) noexcept
{
	const char * src = text->begin();
	const char * send = text->end();
	char * dest = out->end();
	char * dend = out->limit();

	while (src != send)
	{
		char chr = *src++;
		if (chr >= 0x20 && chr != '=' && chr != '&')
		{
			if (dest == dend)
			{
				src--;
				break;
			}
			*dest++ = chr;
		}
		else
		{
			if (dest + 3 > dend)
			{
				src--;
				break;
			}
			*dest++ = '%';
			*dest++ = mem::inttochr(chr & 0xf);
			*dest++ = mem::inttochr(chr >> 4);
		}
	}

	*out = Writer(dest, dend);
	*text = Text(src, send);
}
size_t Uri::encode(char * out, Text text) noexcept
{
	const char * src = text.begin();
	const char * send = text.end();
	char * dest = out;

	while (src != send)
	{
		char chr = *src++;
		if (chr >= 0x20 && chr != '=' && chr != '&')
		{
			*dest++ = chr;
		}
		else
		{
			*dest++ = '%';
			*dest++ = mem::inttochr(chr & 0xf);
			*dest++ = mem::inttochr(chr >> 4);
		}
	}
	return dest - out;
}

size_t Hex::length(Text text) noexcept
{
	return text.size() * 2;
}
void Hex::encode(Writer * out, Text * text) noexcept
{
	const char * src = text->begin();
	const char * send = text->end();
	char * dest = out->end();
	char * dend = out->limit();

	while (src != send)
	{
		byte chr = *src++;
		if (dest + 2 > dend)
		{
			src--;
			break;
		}
		*dest++ = mem::inttochr(chr >> 4);
		*dest++ = mem::inttochr(chr & 0xf);
	}

	*out = Writer(dest, dend);
	*text = Text(src, send);
}
size_t Hex::encode(char * out, Text text) noexcept
{
	const char * src = text.begin();
	const char * send = text.end();
	char * dest = out;

	while (src != send)
	{
		byte chr = *src++;
		*dest++ = mem::inttochr(chr >> 4);
		*dest++ = mem::inttochr(chr & 0xf);
	}
	return dest - out;
}

size_t Base64::length(Text text) noexcept
{
	return (text.size() + 2) / 3 * 4;
}
void Base64::encode(Writer *out, Text * text) noexcept
{
	size_t writed = encode64(out->end(), out->left(), text->begin(), text->size());
	out->commit(writed);
}
size_t Base64::encode(char *out, Text text) noexcept
{
	return encode64(out, text.begin(), text.size());
}
size_t Base64::delength(Text text) noexcept
{
	return decode64_len(text.begin(), text.size());
}
size_t Base64::decode(char *out, Text text) noexcept
{
	return decode64(out, text.begin(), text.size());
}
void Base64::decode(Writer *out, Text * text) noexcept
{
	size_t writed = decode64(out->end(), out->left(), text->begin(), text->size());
	out->commit(writed);
}

size_t Sha1::length(Text text) noexcept
{
	static_assert(SIZE == SHA1HashSize, "Invalid size");
	return SIZE;
}
size_t Sha1::encode(char *out, Text text) noexcept
{
	CHashSHA1 sha1;
	sha1.Reset();
	sha1.Input((const byte *)text.begin(), text.size());
	sha1.Result((byte*)out);
	return SIZE;
}
void Sha1::encode(Writer *out, Text * text) noexcept
{
	if (out->left() < SIZE)
		return;
	encode(out->prepareForce(SIZE), *text);
}
void Sha1::encode2(Writer *out, Text * text) noexcept
{
	size_t sz = out->left();
	if (sz < SHA1HashSize)
		return;

	byte* dest = (byte*)out->end();
	CHashSHA1 sha1;
	sha1.Reset();
	sha1.Input((const byte *)text->begin(), text->size());
	sha1.Result(dest);
	sha1.Reset();
	sha1.Input(dest, SHA1HashSize);
	sha1.Result(dest);
	out->commit(SHA1HashSize);
}

size_t Sha256::length(Text text) noexcept
{
	static_assert(SIZE == SHA256_BLOCK_SIZE, "Invalid size");
	return SIZE;
}
size_t Sha256::encode(char *out, Text text) noexcept
{
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, (const byte *)text.data(), text.size());
	sha256_final(&ctx, (byte*)out);
	return SIZE;
}
void Sha256::encode(Writer *out, Text * text) noexcept
{
	if (out->left() < SIZE)
		return;
	encode(out->prepareForce(SIZE), *text);
}

size_t Md5::length(Text text) noexcept
{
	return SIZE;
}
size_t Md5::encode(char * out, Text text) noexcept
{
	md5_context ctx;
	md5_starts(&ctx);
	md5_update(&ctx, (uint8_t*)text.begin(), intact<uint32_t>(text.size()));
	md5_finish(&ctx, (uint8_t*)out);
	return SIZE;
}
void Md5::encode(Writer * out, Text * text) noexcept
{
	if (out->left() < SIZE)
		return;
	encode(out->prepareForce(SIZE), *text);
}
