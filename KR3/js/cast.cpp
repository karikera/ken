#include "stdafx.h"
#include "cast.h"
#include "rawdata.h"
#include "news.h"
#include "object.h"

#include "cast.inl"

using namespace kr;


_pri_::JsCast::long_to_what_t _pri_::JsCast::toInner(long _value) noexcept
{
	return (long_to_what_t)_value;
}
TText16 _pri_::JsCast::toInner(Text _value, Charset cs) noexcept
{
	TText16 utf16;
	CHARSET_CONSTLIZE(cs, {
		utf16 << (MultiByteToUtf16<cs>)_value;
	});
	return utf16;
}
TText16 _pri_::JsCast::toInner(const AText& _value, Charset cs) noexcept
{
	return toInner((Text)_value, cs);
}
TText16 _pri_::JsCast::toInner(const TText& _value, Charset cs) noexcept
{
	return toInner((Text)_value, cs);
}
Text16 _pri_::JsCast::toInner(const AText16& _value) noexcept
{
	return _value;
}
Text16 _pri_::JsCast::toInner(const TText16& _value) noexcept
{
	return _value;
}
JsObjectRawData _pri_::JsCast::toInner(JsNewObject_t) noexcept
{
	return (JsRawData)JsNewObject;
}
JsObjectRawData _pri_::JsCast::toInner(JsNewArray arr) noexcept
{
	return (JsRawData)arr;
}
JsArrayBufferRawData _pri_::JsCast::toInner(JsNewArrayBuffer arr) noexcept
{
	return (JsRawData)arr;
}
JsTypedArrayRawData _pri_::JsCast::toInner(JsNewTypedArray arr) noexcept
{
	return (JsRawData)arr;
}
JsRawData _pri_::JsCast::toInner(const JsPersistent& value) noexcept
{
	return (JsRawData)value;
}
JsObjectRawData _pri_::JsCast::toInner(JsObject* object) noexcept
{
	return (JsRawData)object->m_data;
}


#define DEFAULT(type, v)	template <> type _pri_::JsCast::defaultValue<type>() noexcept { return v; }
DEFAULT(int, 0);
DEFAULT(double, NAN);
DEFAULT(bool, false);
DEFAULT(nullptr_t, nullptr);
DEFAULT(undefined_t, undefined);
DEFAULT(Text16, u"");
#undef DEFAULT

template <>
JsRawData _pri_::JsCast::defaultValue<JsRawData>() noexcept
{
	return (JsRawData)undefined;
}
template <>
AText16 _pri_::JsCast::defaultValue<AText16>() noexcept
{
	return nullptr;
}
template <>
AText _pri_::JsCast::defaultValue<AText>() noexcept
{
	return nullptr;
}
template <>
Text _pri_::JsCast::defaultValue<Text>() noexcept
{
	return "";
}
template <>
void _pri_::JsCast::defaultValue<void>() noexcept
{
}
