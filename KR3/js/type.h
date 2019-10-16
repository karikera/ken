#pragma once

#include <initializer_list>
#include <KR3/main.h>

namespace kr
{
	namespace _pri_
	{
		struct InternalTools;
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

	class JsRawData;
	class JsPersistent;
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
	class JsIndexAccessor;
	class JsFilter;
	class JsFieldMaker;

	template <typename T>
	class JsField;
	class JsClass;
	typedef JsValue(*JSCallback)(const JsArguments&);
	typedef View<JsValue> JsArgumentsIn;

	enum undefined_t;

	template <typename FUNC>
	class JsFunctionT;

	enum JsNewObject_t { JsNewObject };
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