#include "stdafx.h"
#include "value.h"
#include "value.inl"
#include "string.h"
#include "object.h"
#include "persistent.h"

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
JsRawData _pri_::JsCast::toInner(JsNewObject_t) noexcept
{
	return (JsRawData)JsNewObject;
}
JsRawData _pri_::JsCast::toInner(JsNewArray arr) noexcept
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

JsValue::JsValue() noexcept
{
}
JsValue::JsValue(const JsValue& v) noexcept
	:m_data(v.m_data)
{
}
JsValue::JsValue(JsValue&& v) noexcept
	:m_data(move(v.m_data))
{
}
JsValue::JsValue(const JsRawData& v) noexcept
	:m_data(v)
{
}
JsValue::JsValue(JsRawData&& v) noexcept
	:m_data(move(v))
{
}
JsValue::JsValue(Text text, Charset cs) noexcept
	:m_data(text, cs)
{
}
JsValue::~JsValue() noexcept
{
}

bool JsValue::instanceOf(const JsValue& value) const noexcept
{
	return m_data.instanceOf(value.m_data);
}

bool JsValue::isEmpty() const noexcept
{
	return m_data.isEmpty();
}
void JsValue::set(Text16 name, const JsValue& value) const noexcept
{
	m_data.setProperty(name, value.m_data);
}
JsValue JsValue::get(Text16 name) const noexcept
{
	return m_data.getProperty(name);
}
void JsValue::set(const JsPropertyId& name, const JsValue& value) const noexcept
{
	m_data.setProperty(name, value.m_data);
}
JsValue JsValue::get(const JsPropertyId& name) const noexcept
{
	return m_data.getProperty(name);
}
JsValue JsValue::call(JsValue _this, JsArgumentsIn arguments) const noexcept
{
	return m_data.call(_this.rawdata(), arguments);
}
WBuffer JsValue::getBuffer() const noexcept
{
	JsTypedArrayType type;
	switch (m_data.getType())
	{
	case JsType::ArrayBuffer: return m_data.getArrayBuffer();
	case JsType::TypedArray: return m_data.getTypedArrayBuffer(&type);
	case JsType::DataView: return m_data.getDataViewBuffer();
	default: return nullptr;
	}
}
JsType JsValue::getType() const noexcept
{
	return m_data.getType();
}
JsObject* JsValue::getNativeObject() const noexcept
{
	if (!m_data.instanceOf(JsObject::classObject.rawdata())) return nullptr;
	return m_data.get<JsObject*>();
}


JsRawData& JsValue::rawdata() noexcept
{
	return m_data;
}
const JsRawData& JsValue::rawdata() const noexcept
{
	return m_data;
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
