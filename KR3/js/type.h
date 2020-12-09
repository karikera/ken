#pragma once

#include <initializer_list>
#include <KR3/main.h>

namespace kr
{
	namespace _pri_
	{
		struct InternalTools;
		struct JsCast;
	}

	enum class JsType
	{
		Undefined,
		Null,
		Boolean,
		Integer,
		Float,
		String,
		Function,
		Object,
		ArrayBuffer,
		TypedArray,
		DataView
	};

	using std::nullptr_t;

	class JsScope;
	class JsRawData;
	class JsPersistent;
	class JsWeak;
	class JsValue;
	class JsString;
	class JsFunction;
	class JsObject;
	class JsPropertyId;

	class JsException;
	class JsContext;
	class JsRuntime;
	class JsArguments;
	class JsAccessor;
	class JsGetter;
	class JsIndexAccessor;
	class JsFilter;
	class JsFieldMaker;

	template <typename T>
	class JsField;
	class JsClass;
	typedef JsValue(*JSCallback)(const JsArguments&);
	typedef View<JsValue> JsArgumentsIn;

	class JsNewArray;
	class JsNewArrayBuffer;
	enum class JsTypedType
	{
		Int8,
		Uint8,
		Uint8Clamped,
		Int16,
		Uint16,
		Int32,
		Uint32,
		Float32,
		Float64,
	};
	class JsNewTypedArray;

	template <JsType type>
	class JsTypedRawData;
	using JsFunctionRawData = JsTypedRawData<JsType::Function>;
	using JsObjectRawData = JsTypedRawData<JsType::Object>;
	using JsArrayBufferRawData = JsTypedRawData<JsType::ArrayBuffer>;
	using JsTypedArrayRawData = JsTypedRawData<JsType::TypedArray>;
	using JsDataViewRawData = JsTypedRawData<JsType::DataView>;

	class undefined_t;

	template <class Class, class Parent = JsObject>
	class JsObjectT;
	template <typename FUNC>
	class JsFunctionT;

	class JsNewObject_t final{};
	constexpr const JsNewObject_t JsNewObject = JsNewObject_t();
}

#ifdef KRJS_USE_V8

namespace v8
{
	class Data;
	class Private;
	class Value;
	class External;
	class Object;
	class ObjectTemplate;
	class Context;
	class Function;
	class FunctionTemplate;
	class Script;
	class Array;

#ifndef KRJS_EASYV8_INTERNAL
	template <typename E> class Handle
	{
		void* m_data;
		Handle() = delete;
	};
	template<class T> class NonCopyablePersistentTraits;
	template<class T, class M = NonCopyablePersistentTraits<T> > class Persistent
	{
		void* m_data;
		Persistent() = delete;
	};

#endif
}

namespace kr
{
	using JsRawDataValue = V8Handle<v8::Value>;
	using JsRawContext = v8::Persistent<v8::Context>;
	using JsRawExternal = v8::Persistent<v8::External>;
}
#else

typedef void* JsRef;
typedef JsRef JsValueRef;
typedef JsRef JsContextRef;
typedef JsRef JsRuntimeRef;
typedef JsRef JsPropertyIdRef;

namespace kr
{
	using JsRawDataValue = JsValueRef;
	using JsRawContext = JsContextRef;
	using JsRawExternal = JsValueRef;
	using JsRawRuntime = JsRuntimeRef;
	using JsRawPropertyId = JsPropertyIdRef;
	using JsRawPersistent = JsValueRef;
}

#endif

#ifdef KRJS_USE_V8
#define KRJS_EXPORT __declspec(dllimport)
#else
#define KRJS_EXPORT 
#endif

#define JsAssert(var, condition) if (!(condition)) throw kr::JsException(kr::TSZ16() << u"assert(" u#condition u") but " u#var u" = " << var);
