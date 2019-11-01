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
	Map<Text, Text, true> * getEntities() noexcept
	{	
		static Map<Text, Text, true> entities = {
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

size_t Hex::length(Buffer text) noexcept
{
	return text.size() * 2;
}
void Hex::encode(Writer * out, Buffer* text) noexcept
{
	const byte * src = (byte*)text->begin();
	const byte * send = (byte*)text->end();
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
	*text = Buffer(src, send);
}
size_t Hex::encode(char * out, Buffer text) noexcept
{
	const byte * src = (byte*)text.begin();
	const byte * send = (byte*)text.end();
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
	size_t writed = encode64(out->end(), out->remaining(), text->begin(), text->size());
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
	size_t writed = decode64(out->end(), out->remaining(), text->begin(), text->size());
	out->commit(writed);
}

static_assert(sizeof(Sha1Context) == sizeof(Sha1Context), "sha1 context size unmatch");
static_assert(Sha1Context::SIZE == SHA1HashSize, "Sha1 hash size unmatch");
static_assert(sizeof(Sha256Context) == sizeof(SHA256_CTX), "sha256 context size unmatch");
static_assert(Sha256Context::SIZE == SHA256_BLOCK_SIZE, "sha256 hash size unmatch");
static_assert(sizeof(Md5Context) == sizeof(md5_context), "md5 context size unmatch");

Sha1Context::Sha1Context() noexcept
{
	SHA256_BLOCK_SIZE;
	reset();
}
void Sha1Context::reset() noexcept
{
	CHashSHA1* ctx = (CHashSHA1*)this;
	ctx->Reset();
}
void Sha1Context::update(Buffer input) noexcept
{
	CHashSHA1* ctx = (CHashSHA1*)this;
	ctx->Input((const byte*)input.begin(), input.size());
}
void Sha1Context::finish(void* out) const noexcept
{
	CHashSHA1* ctx = (CHashSHA1*)this;
	ctx->Result((byte*)out);
}

Sha256Context::Sha256Context() noexcept
{
	reset();
}
void Sha256Context::reset() noexcept
{
	SHA256_CTX* ctx = (SHA256_CTX*)this;
	sha256_init(ctx);
}
void Sha256Context::update(Buffer input) noexcept
{
	SHA256_CTX* ctx = (SHA256_CTX*)this;
	sha256_update(ctx, (const byte*)input.data(), input.size());
}
void Sha256Context::finish(void* out) const noexcept
{
	SHA256_CTX* ctx = (SHA256_CTX*)this;
	sha256_final(ctx, (byte*)out);
}

Md5Context::Md5Context() noexcept
{
	reset();
}
void Md5Context::reset() noexcept
{
	md5_context* ctx = (md5_context*)this;
	md5_starts(ctx);
}
void Md5Context::update(Buffer input) noexcept
{
	md5_context* ctx = (md5_context*)this;
	md5_update(ctx, (uint8_t*)input.begin(), intact<uint32_t>(input.size()));
}
void Md5Context::finish(void* out) const noexcept
{
	md5_context* ctx = (md5_context*)this;
	md5_finish(ctx, (uint8_t*)out);
}
