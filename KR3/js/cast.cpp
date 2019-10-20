#include "stdafx.h"
#include "cast.h"
#include "rawdata.h"
#include "news.h"
#include "object.h"

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
Text16 _pri_::JsCast::toInner(const AText16& _value) noexcept
{
	return _value;
}
Text16 _pri_::JsCast::toInner(const TText16& _value) noexcept
{
	return _value;
}
JsRawData _pri_::JsCast::toInner(JsNewObject_t) noexcept
{
	return (JsRawData)JsNewObject;
}
JsRawData _pri_::JsCast::toInner(JsNewArray arr) noexcept
{
	return (JsRawData)arr;
}
JsRawData _pri_::JsCast::toInner(JsNewArrayBuffer arr) noexcept
{
	return (JsRawData)arr;
}
JsRawData _pri_::JsCast::toInner(JsNewTypedArray arr) noexcept
{
	return (JsRawData)arr;
}
JsRawData _pri_::JsCast::toInner(const JsPersistent& value) noexcept
{
	return (JsRawData)value;
}
JsRawData _pri_::JsCast::toInner(JsObject* object) noexcept
{
	return *object;
}
