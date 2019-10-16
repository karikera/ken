#include "stdafx.h"
#include "rawdata.h"
#include "undefined.h"

using namespace kr;

JsNewArray::JsNewArray(size_t size) noexcept
	:size(size)
{
}
JsNewTypedArray::JsNewTypedArray(JsTypedArrayType type, size_t size) noexcept
	:type(type), size(size)
{
}

size_t kr::getElementSize(JsTypedArrayType type) noexcept
{
	static const size_t SIZES[] = {
		1,1,1,
		2,2,
		4,4,
		4,
		8
	};
	_assert((int)type >= 0 && (int)type < countof(SIZES));
	return SIZES[(int)type];
}

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

template <>
nullptr_t JsRawData::get<nullptr_t>() const noexcept
{
	return nullptr;
}
template <>
undefined_t JsRawData::get<undefined_t>() const noexcept
{
	return undefined;
}
