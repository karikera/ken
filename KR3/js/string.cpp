#include "stdafx.h"
#include "string.h"

using namespace kr;

JsString::JsString() noexcept
	:JsValue()
{
}
JsString::JsString(nullptr_t) noexcept
	:JsValue()
{
}

JsString::JsString(pcstr16 str, size_t length) noexcept
	:JsString(Text16(str, length))
{
}
JsString::JsString(Text str, Charset cs) noexcept
	:JsValue(str, cs)
{
}
JsString::JsString(Text16 str) noexcept
	:JsValue(str)
{
}
JsString::JsString(int n) noexcept
	: JsString((Text16)(BText16<32>() << n))
{
}
JsString::JsString(double n) noexcept
	: JsString((Text16)(TSZ16() << n))
{
}
JsString::JsString(bool b) noexcept
	:JsString(b ? (Text16)u"true" : (Text16)u"false")
{
}
JsString::JsString(undefined_t) noexcept
	: JsString(u"undefined")
{
}
JsString::JsString(void*) noexcept
	: JsString(u"[[external]]")
{
}
JsString::JsString(const JsValue& value) noexcept
	:JsValue((value.getType() == JsType::String) ? (JsRawData)value : JsRawData(value.cast<AText16>()))
{
}
JsString& JsString::operator =(const JsString& _copy) noexcept
{
	this->~JsString();
	new(this) JsString(_copy);
	return *this;
}
JsString& JsString::operator =(JsString&& _move) noexcept
{
	this->~JsString();
	new(this) JsString(move(_move));
	return *this;
}

Text16 JsString::get() noexcept
{
	return as<Text16>();
}

JsString::operator Text16() noexcept
{
	return get();
}