#pragma once

#include <KR3/main.h>
#include "type.h"
#include "rawdata.h"

namespace kr
{
	template <JsType typevalue> struct GetTypeFromJsType
	{
		using type = JsTypedRawData<typevalue>;
	};
	template <> struct GetTypeFromJsType<JsType::Boolean> { using type = bool; };
	template <> struct GetTypeFromJsType<JsType::Float> { using type = double; };
	template <> struct GetTypeFromJsType<JsType::Integer> { using type = int; };
	template <> struct GetTypeFromJsType<JsType::Null> { using type = nullptr_t; };
	template <> struct GetTypeFromJsType<JsType::Undefined> { using type = undefined_t; };
	template <> struct GetTypeFromJsType<JsType::String> { using type = Text16; };

	//////////////////////////
	/// MACRO BEGIN
#define KRJS_TYPE_CONSTLIZE_SINGLE(typeValue, fn) \
case typeValue: { typedef GetTypeFromJsType<typeValue>::type type; fn; } break;

#define KRJS_TYPE_CONSTLIZE( ... ) {\
switch(getType())\
{\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Boolean, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Float, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Integer, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Null, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::String, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Undefined, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Function, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::Object, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::DataView, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::ArrayBuffer, __VA_ARGS__)\
KRJS_TYPE_CONSTLIZE_SINGLE(JsType::TypedArray, __VA_ARGS__)\
default: _assert(!"Invalid type");\
}\
}
	/// MACRO END
	//////////////////////////

	// 자바스크립트 변수를 읽기 위한 클래스
	class JsValue
	{
	public:
		JsValue() noexcept;
		template <typename T> JsValue(const T& value) noexcept;
		template <typename T> JsValue(T&& value) noexcept;
		JsValue(const JsValue& v) noexcept;
		JsValue(JsValue&& v) noexcept;
		JsValue(const JsRawData& v) noexcept;
		JsValue(JsRawData&& v) noexcept;
		JsValue(Text text, Charset cs = Charset::Default) noexcept;
		~JsValue() noexcept;

		bool isEmpty() const noexcept;
		bool instanceOf(const JsValue& value) const noexcept;
		void set(Text16 name, const JsValue& value) const noexcept;
		JsValue get(Text16 name) const noexcept;
		void set(const JsPropertyId& name, const JsValue& value) const noexcept;
		JsValue get(const JsPropertyId& name) const noexcept;
		JsValue call(JsValue _this, JsArgumentsIn arguments) const noexcept;

		// for ArrayBuffer, TypedArray, DataView
		WBuffer getBuffer() const noexcept;

		JsRawData& rawdata() noexcept;
		const JsRawData& rawdata() const noexcept;

		// 변수의 타입을 가져온다.
		JsType getType() const noexcept;
		JsObject* getNativeObject() const noexcept;

		// 네이티브 객체를 타입을 정해서 가져옵니다
		// 해당 타입과 맞지 않으면 NULL이 나옵니다
		template <typename T>
		T* getNativeObject() const noexcept
		{
			return dynamic_cast<T*>(getNativeObject());
		}
		template <typename LAMBDA>
		void setMethod(Text16 name, LAMBDA func) const noexcept
		{
			set(name, JsFunction::makeT(func));
		}

		template <typename T> T cast() const noexcept;

		template <>
		void cast<void>() const noexcept;

		// 내부에서 타입 확인 없이, 강제로 캐스트 하여 가져온다.
		template <typename T> T as() const noexcept;

		JsValue& operator =(const JsValue& _copy) noexcept;
		JsValue& operator =(JsValue&& _move) noexcept;

	private:
		JsRawData m_data;

		template <typename T>
		void _ctor(const T& value) noexcept;
		template <typename T>
		void _ctor(T&& value) noexcept;
	};
}

template <>
void kr::JsValue::cast<void>() const noexcept;

template <> 
kr::JsValue kr::JsValue::cast<kr::JsValue>() const noexcept;