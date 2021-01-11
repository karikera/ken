#include "stdafx.h"
#include "value.h"
#include "object.h"

#include "value.inl"

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

void JsValue::set(const JsValue& name, const JsValue& value) const noexcept
{
	return setByIndex(name, value);
}
JsValue JsValue::get(const JsValue& name) const noexcept
{
	return getByIndex(name);
}
void JsValue::set(const JsPropertyId& name, const JsValue& value) const noexcept
{
	return setByProperty(name, value);
}
JsValue JsValue::get(const JsPropertyId& name) const noexcept
{
	return getByProperty(name);
}
JsValue JsValue::getConstructor() const noexcept
{
	return JsRawData::getConstructor();
}
JsValue JsValue::callRaw(JsValue _this, JsArgumentsIn arguments) const throws(JsException)
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
	JsTypedType type;
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
