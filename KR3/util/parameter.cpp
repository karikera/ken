#include "stdafx.h"
#include "parameter.h"

#include <KR3/meta/text.h>

using namespace kr;

PrefixMap::PrefixMap() noexcept
{
	m_array.resize(ONEMAP_SIZE);
	mema::zero(m_array.data(), ONEMAP_SIZE);
}

template <typename C>
ParameterTokenizerT<C>::ParameterTokenizerT(const C * params) noexcept
{
	m_params = params;
}
template <typename C>
Array<C> ParameterTokenizerT<C>::nextString() throws(NotFoundException)
{
	if (m_params == nullptr) throw NotFoundException();

	Array<C> out;
	const C * i = m_params;
	const C * ni;
	if (*i == '\"')
	{
		i++;

		for (;;)
		{
			C chr = *i++;
			if (chr == (C)'\0')
			{
				out.copy(View<C>(m_params + 1, i - m_params - 2));
				ni = nullptr;
				break;
			}
			if (chr == (C)'\"')
			{
				out.copy(View<C>(m_params + 1, i - m_params - 2));
				if (*i == ' ') i++;
				ni = i;
				break;
			}
		}
	}
	else
	{
		for (;;)
		{
			C chr = *i++;
			if (chr == (C)'\0')
			{
				ni = nullptr;
				break;
			}
			if (chr == (C)' ')
			{
				ni = i;
				break;
			}
		}
		out.copy(View<C>(m_params, i - m_params - 1));
	}
	m_params = ni;
	return out;
}
template <typename C>
int ParameterTokenizerT<C>::nextInt() throws(NotFoundException)
{
	if (m_params == nullptr) throw NotFoundException();
	const C * i = m_params;
	int number = 0;
	for (;;)
	{
		C chr = *i++;
		if (chr == ' ')
		{
			m_params = i;
			break;
		}
		if (chr == '\0')
		{
			m_params = nullptr;
			break;
		}
		number *= 10;
		number += chr - '0';
	}
	return number;
}
template <typename C>
bool ParameterTokenizerT<C>::hasNext() noexcept
{
	return m_params != nullptr;
}

template <typename Component>
ParameterT<Component>::ParameterT(std::initializer_list<ParamInfo> paramNames) noexcept
{
	mema::zero(m_shortCut);
	for (const ParamInfo &param : paramNames)
	{
		if (param.shortCut != 0)
			m_shortCut[PrefixMap::toIndex(param.shortCut)] = param.name;

		m_map.insert(param.name, param);
		switch (param.type)
		{
		case ParamNoValue:
			break;
		case ParamValue:
			break;
		case ParamPrefix:
			m_prefix.addText(param.name);
			break;
		}
	}
}
template <typename Component>
ParameterT<Component>::~ParameterT() noexcept
{
}
template <typename Component>
void ParameterT<Component>::start(int argn, Component ** args) noexcept
{
	m_begin = args;
	m_iter = args;
	m_end = args + argn;
	_get(); // exe name
}
template <typename Component>
typename ParameterT<Component>::Value ParameterT<Component>::next() throws(EofException, InvalidSourceException)
{
	static const auto emptyName = meta::literal_as<Component>("");
	Text str = _get();

	Value v;
	switch (str[0])
	{
	case '-':
		if (str.size() >= 1 && (str[1] == '-')) // --name [value]
		{
			str += 2;
			const Component * value = str.find('=');
			if (value != nullptr)
			{
				v.name = str.cut(value);
				v.value = str.subarr(value + 1);
			}
			else
			{
				v.name = str;
				v.value = _getValue(str);
			}
			_checkName(v.name);
		}
		else // -N [value]
		{
			Component shortcut = str[1];
			str += 2;
			if (!str.empty())
			{
				v.name = _checkShortCut(shortcut);
				v.value = str;
			}
			else
			{
				_checkName(str);
				v.name = str;
				v.value = _getValue(str);
			}
		}
		break;
	case '/':
		str++;

		{
			Text paramName = str;
			auto iter = m_map.find(paramName);
			if (iter != m_map.end())
			{
				v.name = paramName;
				v.value = _getValue(iter->second);
				break;
			}

			try
			{
				View<Component> next = m_prefix.find(paramName);
				Text name = paramName.cut(next.data());
				str = next;
				v.name = name;
			}
			catch (NotFoundException&)
			{
				const Component * value = paramName.find(':');
				if (value != nullptr)
				{
					v.name = paramName.cut(value);
					str.subarr_self(value+1);
				}
				else
				{
					v.name = paramName;
					str = emptyName;
				}
				_checkName(v.name);
			}
			if (!str.empty())
			{
				v.value = str;
			}
			else
			{
				StandardErrorOutputT<Component>::out << meta::literal_as<Component>("invalid parameter ") << paramName << (Component)'\n';
				printPosition();
				throw InvalidSourceException();
			}
		}
		break;
	default:
		v.name = emptyName;
		v.value = str;
		break;
	}
	return v;
}
template <typename Component>
void ParameterT<Component>::printPosition() noexcept
{
	using C = Component;
	auto & out = StandardErrorOutputT<C>::out;
	using meta::literal_as;
	using mem = memt<sizeof(C)>;

	out << literal_as<C>("cmd:");
	for(C ** args = m_begin;args != m_end; args++)
	{
		out << (C)' '<< *args;
	}
	out << (C)'\n';

	size_t whiteSpace = 5;
	C ** target = m_iter;
	if (m_iter != m_begin)
		target--;
	for (C ** args = m_begin; args != target; args++)
	{
		whiteSpace++;
		whiteSpace += mem::strlen(*args);
	}
	out.writeFill((C)' ', whiteSpace);
	out << (C)'^' << (C)'\n';
}

template <typename Component>
View<Component> ParameterT<Component>::_getValue(Text name) throws(EofException, InvalidSourceException)
{
	auto iter = m_map.find(name);
	if (iter == m_map.end())
	{
		StandardErrorOutputT<Component>::out << meta::literal_as<Component>("invalid parameter ") << name << (Component)'\n';
		printPosition();
		throw InvalidSourceException();
	}
	return _getValue(iter->second);
}
template <typename Component>
View<Component> ParameterT<Component>::_getValue(const ParamInfo &info) throws(EofException, InvalidSourceException)
{
	try
	{
		static const auto text_noValue = meta::literal_as<Component>("");
		switch (info.type)
		{
		case ParamNoValue:
			return text_noValue;
		default:
			return _get();
		}
	}
	catch (EofException&)
	{
		StandardErrorOutputT<Component>::out << info.name << meta::literal_as<Component>(" parameter need param\n");
		printPosition();
		throw InvalidSourceException();
	}
}
template <typename Component>
View<Component> ParameterT<Component>::_get() throws(EofException)
{
	for (;;)
	{
		if (m_iter == m_end)
			throw EofException();
		Text text = (Text)*m_iter++;
		if (text.empty())
			continue;
		return text;
	}
}
template <typename Component>
void ParameterT<Component>::_checkName(Text name) throws(InvalidSourceException)
{
	auto iter = m_map.find(name);
	if (iter == m_map.end())
	{
		StandardErrorOutputT<Component>::out << meta::literal_as<Component>("invalid parameter ") << name << (Component)'\n';
		printPosition();
		throw InvalidSourceException();
	}
}
template <typename Component>
View<Component> ParameterT<Component>::_checkShortCut(Component shortCut) throws(InvalidSourceException)
{
	try
	{
		Text name = m_shortCut[PrefixMap::toIndex(shortCut)];
		if (name == nullptr)
		{
			StandardErrorOutputT<Component>::out << meta::literal_as<Component>("invalid shortcut ") << shortCut << (Component)'\n';
			throw InvalidSourceException();
		}
		_checkName(name);
		return name;
	}
	catch (NotFoundException&)
	{
		StandardErrorOutputT<Component>::out << meta::literal_as<Component>("invalid character ") << shortCut << (Component)'\n';
		throw InvalidSourceException();
	}
}

template <typename C>
inline TmpArray<C> _readArgument(View<C>* line) noexcept
{
	TmpArray<C> out;
	out.reserve(256);

	C chr;
	const C* str = line->begin();
	const C* end = line->end();
	for (;; str++)
	{
	_out_quot:;
		if (str == end) break;

		chr = *str;
		switch (chr)
		{
		case '\"':
			str++;
			for (;; str++)
			{
				if (str == end) goto _fin;
				chr = *str;
				switch (chr)
				{
				case '"':
					if (str[-1] == '\\')
					{
						out.back() = '"';
						continue;
					}
					str++;
					if (*str == '"')
					{
						out << '"';
						continue;
					}
					goto _out_quot;
				default:
					out << chr;
					break;
				}
			}
			break;
		case ' ': 
			str++;
			while (*str == ' ') str++;
			goto _fin;
		default:
			out << chr;
			break;
		}
	}

_fin:
	line->setBegin(str);
	return out;
}
template <typename C>
inline TmpArray<C> _readArgument(C** pstr) noexcept
{
	TmpArray<C> out;
	out.reserve(256);

	C chr;
	C* str = *pstr;
	for (;; str++)
	{
	_out_quot:;
		chr = *str;
		switch (chr)
		{
		case '\0': goto _fin;
		case '\"':
			str++;
			for (;; str++)
			{
				chr = *str;
				switch (chr)
				{
				case '\0': goto _fin;
				case '"':
					if (str[-1] == '\\')
					{
						out.back() = '"';
						continue;
					}
					str++;
					if (*str == '"')
					{
						out << '"';
						continue;
					}
					goto _out_quot;
				default:
					out << chr;
					break;
				}
			}
			break;
		case ' ': 
			str++; 
			while (*str == ' ') str++;
			goto _fin;
		default:
			out << chr;
			break;
		}
	}
_fin:
	*pstr = str;
	return out;
}

TText kr::readArgument(Text* line) noexcept
{
	return _readArgument(line);
}
TText16 kr::readArgument(Text16* line) noexcept
{
	return _readArgument(line);
}

TText kr::readArgument(char** text) noexcept
{
	return _readArgument(text);
}
TText16 kr::readArgument(char16** text) noexcept
{
	return _readArgument(text);
}


DEFINE_FULL_CHAR_CLASS(kr::ParameterTokenizer);
DEFINE_FULL_CHAR_CLASS(kr::Parameter);
