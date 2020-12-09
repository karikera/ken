#include "stdafx.h"
#include "rawdata.h"
#include "undefined.h"

using namespace kr;

JsRawData::~JsRawData() noexcept
{

}
JsRawData::JsRawData(const JsRawData& data) noexcept
	:JsRawData(data.m_data)
{
}
JsRawData::JsRawData(JsRawData&& data) noexcept
	:JsRawData(move(data.m_data))
{
}
JsRawData& JsRawData::operator =(const JsRawData& data) noexcept
{
	this->~JsRawData();
	new(this) JsRawData(data);
	return *this;
}
JsRawData& JsRawData::operator =(JsRawData&& data) noexcept
{
	this->~JsRawData();
	new(this) JsRawData(move(data));
	return *this;
}

JsRawData::JsRawData(Text text, Charset cs) noexcept
{
	CHARSET_CONSTLIZE(cs, {
		TSZ16 utf16;
		utf16 << (MultiByteToUtf16<cs>)text;
		new(this) JsRawData((Text16)utf16);
	});
}
WBuffer JsRawData::getTypedArrayBuffer() const noexcept
{
	JsTypedType type;
	return getTypedArrayBuffer(&type);
}

template <>
nullptr_t JsRawData::as<nullptr_t>() const noexcept
{
	return nullptr;
}
template <>
undefined_t JsRawData::as<undefined_t>() const noexcept
{
	return undefined;
}
