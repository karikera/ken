#include "stdafx.h"
#include "jsonparser.h"

using namespace kr;

JsonParser::JsonParser(io::VIStream<char> is) noexcept
	:Parser(is)
{
	m_closeCharacter = -1;
}
JsonParser::~JsonParser() noexcept
{
}
JsonParser::Type JsonParser::getNextType() throws(InvalidSourceException)
{
	skipWhiteSpace();
	switch (m_is.peek())
	{
	case '-':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return Number;
	case '\"':
	case '\'':
		return String;
	case '{':
		return Object;
	case '[':
		return Array;
	case 'n':
		return Null;
	case 't':
	case 'f':
		return Boolean;
	default:
		throw InvalidSourceException();
	}
}
float JsonParser::floatNumber() throws(InvalidSourceException)
{
	skipWhiteSpace();

	bool negative;

	char chr = m_is.peek();
	if (chr == '-')
	{
		m_is.skip(1);
		negative = true;
	}
	else
		negative = false;

	TText tx((size_t)0, 256);
	m_is.readto_F(&tx, [&](Text text)->const char* {
		const char * src = text.begin();
		const char * esrc = text.end();
		for (; src != esrc; src++)
		{
			if ('0' <= *src && *src <= '9') continue;
			if (*src == '.') continue;
			return src;
		}
		return nullptr;
	});

	_closeTest();
	return tx.to_float();
}
int JsonParser::integer() throws(InvalidSourceException)
{
	uint radix;
	bool negative;

	TText tx((size_t)0, 256);
	_readNumber(&tx, &radix, &negative);

	if (negative)
	{
		return -(int)tx.to_uint(radix);
	}
	else
	{
		return tx.to_uint(radix);
	}
}
int64_t JsonParser::integer64() throws(InvalidSourceException)
{
	uint radix;
	bool negative;

	TText tx((size_t)0, 256);
	_readNumber(&tx, &radix, &negative);

	if (negative)
	{
		return -(int64_t)tx.to_uint64(radix);
	}
	else
	{
		return tx.to_uint64(radix);
	}
}
uint JsonParser::uinteger() throws(InvalidSourceException)
{
	uint radix;
	bool negative;

	TText tx((size_t)0, 256);
	_readNumber(&tx, &radix, &negative);
	if (negative) return 0;

	return tx.to_uint(radix);
}
uint64_t JsonParser::uinteger64() throws(InvalidSourceException)
{
	uint radix;
	bool negative;

	TText tx((size_t)0, 256);
	_readNumber(&tx, &radix, &negative);
	if (negative) return 0;

	return tx.to_uint64(radix);
}
bool JsonParser::boolean() throws(InvalidSourceException)
{
	skipWhiteSpace();
	if (_nextIs_y("tf") == 't')
	{
		m_is.must("rue");
		_closeTest();
		return true;
	}
	else
	{
		m_is.must("alse");
		_closeTest();
		return false;
	}
}
AText16 JsonParser::text16() throws(InvalidSourceException)
{
	AText16 out;
	out << (Utf8ToUtf16)ttext();
	return out;
}
AText JsonParser::text() throws(InvalidSourceException)
{
	AText out;
	out << ttext();
	return out;
}
TText JsonParser::ttext() throws(InvalidSourceException)
{
	switch (getNextType())
	{
	case Null:
		_nextIsNull();
		return TText();
	case Number: {
		TText tx;
		tx << floatNumber();
		return tx;
	}
	case String: {
		char chr = _nextIs_y("\"\'");
		TText tx = readWithUnslash(chr);
		_closeTest();
		return tx;
	}
	case Boolean:
		return boolean() ? (TText)"true" : (TText)"false";
	case Array:
		skipValue();
		return "[array]";
	default: // Object
		skipValue();
		return "[object]";
	}
}
void JsonParser::skipValue() throws(InvalidSourceException)
{
	skipWhiteSpace();
	char peeked = m_is.peek();
	switch (peeked)
	{
	case '\'':
	case '\"':
		skipWithUnslash(_nextIs_y("\"\'"));
		_closeTest();
		break;
	case '{':
		object([this](Text name) { skipValue(); });
		break;
	case '[':
		array([this](size_t) { skipValue(); });
		break;
	default:
		if ('0' <= peeked && peeked <= '9')
		{
			_skipNumber();
		}
		else
		{
			try
			{
				switch (m_is.read())
				{
				case 't': m_is.must("rue"); break;
				case 'f': m_is.must("alse"); break;
				case 'n': m_is.must("ull"); break;
				}
			}
			catch (EofException&)
			{
				throw InvalidSourceException();
			}
			_closeTest();
		}
		break;
	}
}

char JsonParser::_open(char closechr) throws(InvalidSourceException)
{
	char oldchr = m_closeCharacter;
	skipWhiteSpace();
	if (m_is.peek() == closechr)
	{
		m_is.read();
		m_closeCharacter = 0;
	}
	else
		m_closeCharacter = closechr;
	return oldchr;
}
TText JsonParser::_getName() throws(InvalidSourceException)
{
	char chr = _nextIs_y("\"\'");
	TText tx = readWithUnslash(chr);
	_nextIs(':');
	return tx;
}

void JsonParser::_readNumber(TText * dest, uint * radix, bool * negative) throws(InvalidSourceException)
{
	char isString = '\0';
	{
		skipWhiteSpace();
		switch (m_is.peek())
		{
		case '-':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			break;
		case '\"':
		case '\'':
			isString = m_is.read();
			break;
		case '{':
			skipValue();
			*dest << '1';
			*negative = true;
			*radix = 16;
			return;
		case '[':
			skipValue();
			*dest << '1';
			*negative = true;
			*radix = 16;
			return;
		case 'n':
			m_is.skip();
			m_is.must("ull");
			*dest << '0';
			*negative = false;
			*radix = 16;
			return;
		default:
			throw InvalidSourceException();
		}

		char chr = m_is.peek();
		if (chr == '-')
		{
			m_is.skip(1);
			chr = m_is.peek();
			*negative = true;
		}
		else
		{
			*negative = false;
		}
		if (chr == '0')
		{
			m_is.skip(1);
			chr = m_is.peek();
			switch (chr)
			{
			case 'X':
			case 'x':
				m_is.skip(1);
				*radix = 16;
				break;
			case '0':case '1':case '2':case '3':case '4':
			case '5':case '6':case '7':case '8':case '9':
				*radix = 8;
				break;
			default:
				*dest << '0';
				_closeTest();
				return;
			}
		}
		else if ('1' <= chr && chr <= '9')
		{
			*radix = 10;
		}
		else if (isString)
		{
			*dest << '1';
			*negative = true;
			*radix = 16;
			return;
		}
		else
		{
			throw InvalidSourceException();
		}
	}

	char maxchr = *radix >= 10 ? ('9' + 1) : (char)('0' + *radix);
	char maxchrUpper = 'A' + (*radix - 10);
	char maxchrLower = 'a' + (*radix - 10);
	m_is.readto_F(dest, [&](Text text)->const char* {
		const char * src = text.begin();
		const char * esrc = text.end();
		for (; src != esrc; src++)
		{
			if ('0' <= *src && *src < maxchr) continue;
			if (*radix > 10)
			{
				if ('A' <= *src && *src <= maxchrUpper) continue;
				if ('a' <= *src && *src <= maxchrLower) continue;
			}
			return src;
		}
		return nullptr;
	});
	if (isString) skipWithUnslash(isString);
	_closeTest();
}
void JsonParser::_closeTest() throws(InvalidSourceException)
{
	skipWhiteSpace();
	try
	{
		char last = m_is.read();
		if (last == ',')
		{
			return;
		}
		else if (last == m_closeCharacter)
		{
			m_closeCharacter = 0;
			return;
		}
	}
	catch (EofException&)
	{
		if (m_closeCharacter == -1)
		{
			m_closeCharacter = 0;
			return;
		}
	}
	throw InvalidSourceException();
}

void JsonParser::_nextIsNull() throws(InvalidSourceException)
{
	skipWhiteSpace();
	m_is.must("null");
	_closeTest();
}
void JsonParser::_nextIs(char chr) throws(InvalidSourceException)
{
	skipWhiteSpace();
	if (m_is.read() != chr) throw InvalidSourceException();
}
char JsonParser::_nextIs_y(Text chr) throws(InvalidSourceException)
{
	skipWhiteSpace();
	try
	{
		char out = m_is.read();
		if (!chr.contains(out)) throw InvalidSourceException();
		return out;
	}
	catch (EofException&)
	{
		throw InvalidSourceException();
	}
}
void JsonParser::_skipNumber() throws(InvalidSourceException)
{
	try
	{
		uint radix;

		skipWhiteSpace();

		char chr = m_is.read();
		if (chr == '0')
		{
			chr = m_is.peek();
			switch (chr)
			{
			case 'X':
			case 'x':
				m_is.skip(1);
				radix = 16;
				break;
			default:
				radix = 8;
				break;
			}
		}
		else if ('1' <= chr && chr <= '9')
		{
			radix = 10;
		}
		else throw InvalidSourceException();

		char maxchr = radix >= 10 ? ('9' + 1) : (char)('0' + radix);
		char maxchrUpper = 'A' + (radix - 10);
		char maxchrLower = 'a' + (radix - 10);
		m_is.skipto_F([&](Text text)->const char* {
			const char * src = text.begin();
			const char * esrc = text.end();
			for (; src != esrc; src++)
			{
				if ('0' <= *src && *src < maxchr) continue;
				if (radix > 10)
				{
					if ('A' <= *src && *src <= maxchrUpper) continue;
					if ('a' <= *src && *src <= maxchrLower) continue;
				}
				return src;
			}
			return nullptr;
		});
		if (radix == 10)
		{
			if (m_is.nextIs('.'))
			{
				m_is.skipto_F([&](Text text)->const char* {
					const char * src = text.begin();
					const char * esrc = text.end();
					for (; src != esrc; src++)
					{
						if ('0' <= *src && *src <= '9') continue;
						return src;
					}
					return nullptr;
				});
			}
		}

		_closeTest();
	}
	catch (EofException&)
	{
		throw InvalidSourceException();
	}
}
bool JsonParser::_skipIfNot(char chr) throws(InvalidSourceException)
{
	skipWhiteSpace();
	if (m_is.nextIs(chr)) return false;
	skipValue();
	return true;
}
