#pragma once


#include "type.h"

namespace kr
{
	class JsRawData
	{
		friend _pri_::InternalTools;
		friend JsFunction;
		friend JsContext;
		friend JsClass;
		friend JsPersistent;
		friend JsScope;
		friend JsRuntime;
		friend JsException;
	public:
		~JsRawData() noexcept;
		JsRawData(const JsRawData& data) noexcept;
		JsRawData(JsRawData&& data) noexcept;
		JsRawData& operator =(const JsRawData& data) noexcept;
		JsRawData& operator =(JsRawData&& data) noexcept;
		JsRawData(Text text, Charset cs) noexcept;
		KRJS_EXPORT JsRawData() noexcept;
		KRJS_EXPORT explicit JsRawData(const JsPersistent& data) noexcept;
		KRJS_EXPORT explicit JsRawData(Text16 text) noexcept;
		KRJS_EXPORT explicit JsRawData(const JsRawDataValue& data) noexcept;
		KRJS_EXPORT explicit JsRawData(JsRawDataValue&& data) noexcept;
		KRJS_EXPORT explicit JsRawData(int value) noexcept;
		KRJS_EXPORT explicit JsRawData(double value) noexcept;
		KRJS_EXPORT explicit JsRawData(bool value) noexcept;
		KRJS_EXPORT explicit JsRawData(nullptr_t) noexcept;
		KRJS_EXPORT explicit JsRawData(undefined_t) noexcept;
		KRJS_EXPORT explicit JsRawData(JsNewObject_t) noexcept;
		KRJS_EXPORT explicit JsRawData(JsNewArray arr) noexcept;
		KRJS_EXPORT explicit JsRawData(JsNewTypedArray arr) noexcept;
		KRJS_EXPORT explicit JsRawData(JsNewArrayBuffer arr) noexcept;
		KRJS_EXPORT bool isEmpty() const noexcept;
		KRJS_EXPORT JsType getType() const noexcept;
		KRJS_EXPORT void setProperty(Text16 name, const JsRawData &value) const noexcept;
		KRJS_EXPORT void setProperty(const JsPropertyId &name, const JsRawData& value) const noexcept;
		KRJS_EXPORT JsRawData getProperty(Text16 name) const noexcept;
		KRJS_EXPORT JsRawData getProperty(const JsPropertyId& name) const noexcept;
		KRJS_EXPORT bool instanceOf(const JsRawData& value) const noexcept;
		KRJS_EXPORT void* getExternalData() const noexcept;
		KRJS_EXPORT WBuffer getArrayBuffer() const noexcept;
		KRJS_EXPORT WBuffer getDataViewBuffer() const noexcept;
		KRJS_EXPORT WBuffer getTypedArrayBuffer(JsTypedArrayType * type) const noexcept;
		WBuffer getTypedArrayBuffer() const noexcept;
		KRJS_EXPORT JsRawData call(JsRawData _this, JsArgumentsIn arguments) const throws(JsException);
		KRJS_EXPORT bool equals(const JsRawData & other) const noexcept;
		KRJS_EXPORT bool abstractEquals(const JsRawData& other) const noexcept;
		KRJS_EXPORT JsRawData toString() const throws(JsException);

		// get value without any-cast
		// need to match type
		template <typename T>
		T as() const noexcept;
		template <typename LAMBDA>
		void getTypedArrayBufferL(const LAMBDA& onBuffer) noexcept;

	private:
		JsRawDataValue m_data;
	};

	template <JsType type>
	class JsTypedRawData :public JsRawData
	{
	public:
		JsTypedRawData() {}
		JsTypedRawData(const JsRawData& data) noexcept
			:JsRawData(data)
		{
		}
		JsTypedRawData(JsRawData&& data) noexcept
			:JsRawData(move(data))
		{
		}
	};
}


template <>
KRJS_EXPORT kr::Text16 kr::JsRawData::as<kr::Text16>() const noexcept;
template <>
KRJS_EXPORT kr::JsObject* kr::JsRawData::as<kr::JsObject*>() const noexcept;
template <>
KRJS_EXPORT int kr::JsRawData::as<int>() const noexcept;
template <>
KRJS_EXPORT double kr::JsRawData::as<double>() const noexcept;
template <>
KRJS_EXPORT bool kr::JsRawData::as<bool>() const noexcept;

template <>
nullptr_t kr::JsRawData::as<nullptr_t>() const noexcept;
template <>
kr::undefined_t kr::JsRawData::as<kr::undefined_t>() const noexcept;

template <typename T>
T kr::JsRawData::as() const noexcept
{
	return *this;
}

template <typename LAMBDA>
void kr::JsRawData::getTypedArrayBufferL(const LAMBDA& onBuffer) noexcept
{
	JsTypedArrayType type;
	Buffer buffer = getTypedArrayBuffer(&type);
	switch (type)
	{
	case JsTypedArrayType::Int8: onBuffer(buffer.cast<int8_t>()); break;
	case JsTypedArrayType::Uint8: 
	case JsTypedArrayType::Uint8Clamped: onBuffer(buffer.cast<uint8_t>()); break;
	case JsTypedArrayType::Int16: onBuffer(buffer.cast<int16_t>()); break;
	case JsTypedArrayType::Uint16: onBuffer(buffer.cast<uint16_t>()); break;
	case JsTypedArrayType::Int32: onBuffer(buffer.cast<int32_t>()); break;
	case JsTypedArrayType::Uint32: onBuffer(buffer.cast<uint32_t>()); break;
	case JsTypedArrayType::Float32: onBuffer(buffer.cast<float>()); break;
	case JsTypedArrayType::Float64: onBuffer(buffer.cast<double>()); break;
	default: unreachable();
	}
}