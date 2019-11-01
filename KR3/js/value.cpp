#include "stdafx.h"
#include "value.h"
#include "object.h"

using namespace kr;

JsValue::JsValue() noexcept
{
}
JsValue::JsValue(const JsValue& v) noexcept
	:JsRawData(v)
{
}
JsValue::JsValue(JsValue&& v) noexcept
	:JsRawData((JsRawData&&)v)
{
}
JsValue::JsValue(const JsRawData& v) noexcept
	:JsRawData(v)
{
}
JsValue::JsValue(JsRawData&& v) noexcept
	:JsRawData(move(v))
{
}
JsValue::JsValue(Text text, Charset cs) noexcept
	:JsRawData(text, cs)
{
}
JsValue::~JsValue() noexcept
{
}

void JsValue::set(Text16 name, const JsValue& value) const noexcept
{
	return setProperty(name, value);
}
JsValue JsValue::get(Text16 name) const noexcept
{
	return getProperty(name);
}
void JsValue::set(const JsPropertyId& name, const JsValue& value) const noexcept
{
	return setProperty(name, value);
}
JsValue JsValue::get(const JsPropertyId& name) const noexcept
{
	return getProperty(name);
}
JsValue JsValue::call(JsValue _this, JsArgumentsIn arguments) const throws(JsException)
{
	return JsRawData::call(_this, arguments);
}
bool JsValue::operator ==(const JsValue& value) const noexcept
{
	return equals(value);
}
bool JsValue::operator !=(const JsValue& value) const noexcept
{
	return !equals(value);
}
WBuffer JsValue::getBuffer() const noexcept
{
	JsTypedArrayType type;
	switch (getType())
	{
	case JsType::ArrayBuffer: return getArrayBuffer();
	case JsType::TypedArray: return getTypedArrayBuffer(&type);
	case JsType::DataView: return getDataViewBuffer();
	default: return nullptr;
	}
}
JsObject* JsValue::getNativeObject() const noexcept
{
	if (!instanceOf(JsObject::classObject)) return nullptr;
	return as<JsObject*>();
}

template <>
void JsValue::cast<void>() const noexcept
{
}

template <>
kr::JsValue kr::JsValue::cast<kr::JsValue>() const noexcept
{
	return *this;
}

JsValue& JsValue::operator =(const JsValue& _copy) noexcept
{
	this->~JsValue();
	new(this) JsValue(_copy);
	return *this;
}
JsValue& JsValue::operator =(JsValue&& _move) noexcept
{
	this->~JsValue();
	new(this) JsValue(move(_move));
	return *this;
}
