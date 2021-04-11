

#include "stdafx.h"

#ifndef KRJS_USE_V8

#include "js.h"

#include <KR3/util/wide.h>
#include <KR3/util/dump.h>
#include <KR3/data/set.h>

#define USE_EDGEMODE_JSRT
#include <jsrt.h>

#pragma warning(disable:4073)
#pragma warning(disable:26812)
#pragma init_seg(lib)

kr::InitPack kr::JsRuntime::initpack;

using kr::_pri_::InternalTools;

namespace
{	
	kr::JsContext * s_context;

	kr::JsClassT<kr::JsObject> * s_nativeClass;
	JsRuntimeHandle s_runtime;
	JsSourceContext s_sourceContextCounter;

	ondebug(int s_scopeStackCounter);
}

struct _NOERR
{
	_NOERR(JsErrorCode err) noexcept
	{
		_assert(err == JsNoError);
	}
};

struct _ErrorCatchThrow
{
	_ErrorCatchThrow(JsErrorCode err) throws(kr::JsException);
};

// Return Value Check
#ifdef NDEBUG
#define NOERR
#define JsAssertRelease(...)  JsRelease(__VA_ARGS__, nullptr)
#define JsAssertAddRef(...)  JsAddRef(__VA_ARGS__, nullptr)
#else
#define NOERR _NOERR CONCAT(__rvc, __COUNTER__) =
#define JsAssertRelease(...) do { JsValueRef __kr_ref = (__VA_ARGS__); \
	JsErrorCode err = JsRelease(__kr_ref, nullptr); _assert(__kr_ref == JS_INVALID_REFERENCE || err == JsNoError); } while(false)
#define JsAssertAddRef(...) do { JsValueRef __kr_ref = (__VA_ARGS__); \
	JsErrorCode err = JsAddRef(__kr_ref, nullptr); _assert(__kr_ref == JS_INVALID_REFERENCE || err == JsNoError); } while(false)
#endif
#define ERRCT _ErrorCatchThrow CONCAT(__rvc, __COUNTER__) =


namespace kr
{
	namespace _pri_
	{
		struct InternalTools
		{
			template <typename LAMBDA>
			static JsRef wrapToJsException(const LAMBDA& lambda) noexcept
			{
				SEHCatcher __catcher;
				try
				{
					return lambda();
				}
				catch (JsException & e)
				{
					JsSetException(e.m_exception);
					return JS_INVALID_REFERENCE;
				}
				catch (JsValue& value)
				{
					JsSetException(value.m_data);
					return JS_INVALID_REFERENCE;
				}
				catch (SEHException & e)
				{
					JsSetException(createError(TSZ16() << u"Structured Exception: " << e.getErrorText() << u"(0x" << hexf(e.getErrorCode(), 8) << u')').m_data);
					return JS_INVALID_REFERENCE;
				}
				catch (std::exception & e)
				{
					JsSetException(createError(TSZ16() << u"Standard Exception: " << AnsiToUtf16((Text)e.what())).m_data);
					return JS_INVALID_REFERENCE;
				}
				catch (...)
				{
					JsSetException(createError(u"Unknown Exception").m_data);
					return JS_INVALID_REFERENCE;
				}
			}
			template <typename OBJ>
			static JsValue createExternalFunction(JsNativeFunction func, OBJ * nativeobj) noexcept
			{
				ondebug(_assert(s_scopeStackCounter != 0));

				nativeobj->AddRef();
				JsValue out;
				NOERR JsCreateFunction(func, nativeobj, &out.m_data);
				NOERR JsSetObjectBeforeCollectCallback(out.m_data, nativeobj, [](JsRef func, void* state) {
					((OBJ*)state)->Release();
					});
				int ref = getRef(out.m_data);
				return out;
			}
			static JsTypedType typedArrayTypeToKr(::JsTypedArrayType type) noexcept
			{
				static_assert(
					(int)JsTypedType::Int8 == JsArrayTypeInt8 &&
					(int)JsTypedType::Uint8 == JsArrayTypeUint8 &&
					(int)JsTypedType::Uint8Clamped == JsArrayTypeUint8Clamped &&
					(int)JsTypedType::Int16 == JsArrayTypeInt16 &&
					(int)JsTypedType::Uint16 == JsArrayTypeUint16 &&
					(int)JsTypedType::Int32 == JsArrayTypeInt32 &&
					(int)JsTypedType::Uint32 == JsArrayTypeUint32 &&
					(int)JsTypedType::Float32 == JsArrayTypeFloat32 &&
					(int)JsTypedType::Float64 == JsArrayTypeFloat64, "Enum not match");
				//static const JsTypedArrayType retype[] = {
				//	JsTypedArrayType::Int8,
				//	JsTypedArrayType::Uint8,
				//	JsTypedArrayType::Uint8Clamped,
				//	JsTypedArrayType::Int16,
				//	JsTypedArrayType::Uint16,
				//	JsTypedArrayType::Int32,
				//	JsTypedArrayType::Uint32,
				//	JsTypedArrayType::Float32,
				//	JsTypedArrayType::Float64,
				//};
				//_assert(arrayType < countof(retype) && arrayType >= 0);
				//*type = retype[arrayType];
				return (JsTypedType)type;
			}
			static ::JsTypedArrayType typedArrayTypeToJsrt(JsTypedType type) noexcept
			{
				static_assert(
					(int)JsTypedType::Int8 == JsArrayTypeInt8 &&
					(int)JsTypedType::Uint8 == JsArrayTypeUint8 &&
					(int)JsTypedType::Uint8Clamped == JsArrayTypeUint8Clamped &&
					(int)JsTypedType::Int16 == JsArrayTypeInt16 &&
					(int)JsTypedType::Uint16 == JsArrayTypeUint16 &&
					(int)JsTypedType::Int32 == JsArrayTypeInt32 &&
					(int)JsTypedType::Uint32 == JsArrayTypeUint32 &&
					(int)JsTypedType::Float32 == JsArrayTypeFloat32 &&
					(int)JsTypedType::Float64 == JsArrayTypeFloat64, "Enum not match");
				//static const JsTypedArrayType retype[] = {
				//	JsTypedArrayType::Int8,
				//	JsTypedArrayType::Uint8,
				//	JsTypedArrayType::Uint8Clamped,
				//	JsTypedArrayType::Int16,
				//	JsTypedArrayType::Uint16,
				//	JsTypedArrayType::Int32,
				//	JsTypedArrayType::Uint32,
				//	JsTypedArrayType::Float32,
				//	JsTypedArrayType::Float64,
				//};
				//_assert(arrayType < countof(retype) && arrayType >= 0);
				//*type = retype[arrayType];
				return (::JsTypedArrayType)type;
			}
			static JsRawData createError(Text16 message) noexcept
			{
				ondebug(_assert(s_scopeStackCounter != 0));
				JsRawData out;
				NOERR JsCreateError(JsRawData(message).m_data, &out.m_data);
				return out;
			}
			static JsArguments makeArgs(JsRawData _this, JsValueRef* arguments, unsigned short argumentCount) noexcept
			{
				static_assert(sizeof(JsRawDataValue) == sizeof(JsValue), "size unmatched");
				return JsArguments(_this, (JsValue*)arguments + 1, argumentCount - 1);
			}
			static JsValueRef CT_STDCALL nativeConstructorCallback(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState) noexcept
			{
				if (!isConstructCall)
				{
					NOERR JsSetException(createError(u"construct only function").m_data);
					return JS_INVALID_REFERENCE;
				}
				JsScope scope;
				
				JsRawData _thisref;
				NOERR JsCreateExternalObject(nullptr, [](void * data) {
					if (data == nullptr) return;
					((JsObject*)data)->finalize();
				}, & _thisref.m_data);

				JsRawData prototype;
				JsGetPrototype(arguments[0], &prototype.m_data);
				JsSetPrototype(_thisref.m_data, prototype.m_data);

				JsArguments args = makeArgs(_thisref, arguments, argumentCount);

				using CTOR = JsClass::CTOR;
				CTOR ctor = (CTOR)callbackState;
				return wrapToJsException([&] {
					JsObject* jsobj = ctor(args);
					JsSetExternalData(_thisref.m_data, jsobj);
					return _thisref.m_data;
					});
			}
			static JsValueRef CT_STDCALL nativeFunctionCallback(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState) noexcept
			{
				kr::JsFunction::Data*  data = (kr::JsFunction::Data*)callbackState;
				return wrapToJsException([&] {
					JsScope scope;
					JsArguments args = makeArgs((JsRawData)*arguments, arguments, argumentCount);
					JsValue retValue = data->call(args);
					return retValue.m_data;
					});
			}
			static JsValueRef CT_STDCALL getterCallback(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState) noexcept
			{
				kr::JsAccessor* data = (kr::JsAccessor*)callbackState;
				return wrapToJsException([&] {
					JsScope scope;
					JsValue retValue = data->get((JsRawData)arguments[0]);
					return retValue.m_data;
					});
			}
			static JsValueRef CT_STDCALL setterCallback(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState) noexcept
			{
				kr::JsAccessor* data = (kr::JsAccessor*)callbackState;
				return wrapToJsException([&] {
					JsScope scope;
					data->set((JsRawData)arguments[0], argumentCount >= 2 ? (JsValue)(JsRawData)arguments[1] : (JsValue)undefined);
					return JS_INVALID_REFERENCE;
					});
			}

			static JsContextRef createContext() noexcept
			{
				JsContextRef ctx;
				NOERR JsCreateContext(s_runtime, &ctx);
				return ctx;
			}
			static JsContextRef getCurrentContext() noexcept
			{
				JsContextRef oldctx;
				NOERR JsGetCurrentContext(&oldctx);
				return oldctx;
			}
			static bool hasExternalData(JsValueRef object) noexcept
			{
				bool out;
				NOERR JsHasExternalData(object, &out);
				return out;
			}
			
			static JsRawData createClass(JsRawData name, JsClassInfo::CTOR info, JsPropertyIdRef prototypeId, JsPropertyIdRef constructorId) noexcept
			{
				ondebug(_assert(s_scopeStackCounter != 0));
				JsRawData cls;
				NOERR JsCreateNamedFunction(name.m_data, InternalTools::nativeConstructorCallback, info, &cls.m_data);

				JsRawData prototype;
				NOERR JsCreateObject(&prototype.m_data);
				NOERR JsSetProperty(cls.m_data, prototypeId, prototype.m_data, true);
				NOERR JsSetProperty(prototype.m_data, constructorId, cls.m_data, true);
				return cls;
			}
			static void extends(JsRawData child, JsRawData parent, JsPropertyIdRef prototypeId) noexcept
			{
				ondebug(_assert(s_scopeStackCounter != 0));

				JsRawData parentPrototype;
				JsRawData childPrototype;
				NOERR JsGetProperty(parent.m_data, prototypeId, &parentPrototype.m_data);
				NOERR JsGetProperty(child.m_data, prototypeId, &childPrototype.m_data);
				
				NOERR JsSetPrototype(child.m_data, parent.m_data);
				NOERR JsSetPrototype(childPrototype.m_data, parentPrototype.m_data);
			}
			ATTR_NORETURN static void throwException(JsRawException exception) throws(JsException)
			{
				ondebug(_assert(s_scopeStackCounter != 0));

				JsException ex;
				ex.m_exception = exception;
				throw ex;
			}
			static int getRef(JsRef data) noexcept
			{
				uint refcount;
				NOERR JsAddRef(data, &refcount);
				NOERR JsRelease(data, nullptr);
				return refcount - 1;
			}
			static int getRef(JsRawData data) noexcept
			{
				return getRef(data.m_data);
			}
			static int getRef(const JsPersistent& data) noexcept
			{
				return getRef(data.m_data);
			}
			static void test() noexcept
			{
				uint refcount = 0;

				class DtorTest
				{
				private:
					bool* m_dtored;

				public:
					DtorTest(bool* out) noexcept
						:m_dtored(out){
					}

					DtorTest(DtorTest&& _moved) {
						m_dtored = _moved.m_dtored;
						_moved.m_dtored = nullptr;
					}
					~DtorTest() noexcept
					{
						if (m_dtored)
						{
							_assert(*m_dtored == false);
							*m_dtored = true;
						}
					}
				};
				JsValue func;
				JsPersistent saved;
				{
					JsScope scope;
					func = JsFunction::makeT([&] {
						JsValue test = JsNewObject;
						uint first = getRef(test);
						saved = test;
						refcount = getRef(test);
						_assert(first + 1 == refcount);
						return test;
						});
					JsValue test = func();
					int funcref = getRef(func);
					int nref = getRef(test);
					_assert(nref == refcount);
					JsRelease(test.m_data, nullptr);
				}
				int funcref = getRef(func);
				{
					int nref = getRef(saved);
					_assert(nref == refcount - 1);
				}

				JsRuntime::gc();
			}

			static void throwException(JsErrorCode err) throws(kr::JsException)
			{
				throw kr::JsException(kr::TSZ16() << u"JsErrorCode: 0x" << kr::hexf((int)err));
			}
		};
	}
}

_ErrorCatchThrow::_ErrorCatchThrow(JsErrorCode err) throws(kr::JsException)
{
	if (err == JsNoError) return;
	JsValueRef exception;
	if (JsGetAndClearException(&exception) == JsNoError)
	{
		InternalTools::throwException(exception);
	}
	else
	{
		InternalTools::throwException(err);
	}
}

kr::JsExceptionCatcher::JsExceptionCatcher() noexcept
{
}
kr::JsExceptionCatcher::~JsExceptionCatcher() throws(kr::JsException)
{
	JsValueRef exception;
	if (JsGetAndClearException(&exception) == JsNoError) {
		InternalTools::throwException(exception);
	}
}

// scope
kr::JsScope::JsScope() noexcept
{
	_assert(InternalTools::getCurrentContext() != nullptr);
	ondebug(s_scopeStackCounter++);
}
kr::JsScope::~JsScope() noexcept
{
	ondebug(s_scopeStackCounter--);
}

// rawdata
kr::JsRawData::JsRawData() noexcept
{
	m_data = JS_INVALID_REFERENCE;
}
kr::JsRawData::JsRawData(Text16 text) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	if (text == nullptr) // JsPointerToString nullptr 0 is fail
	{
		NOERR JsPointerToString(L"", 0, &m_data);
	}
	else
	{
		NOERR JsPointerToString(wide(text.data()), text.size(), &m_data);
	}
}
kr::JsRawData::JsRawData(const JsPersistent& data) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	m_data = data.m_data;
}
kr::JsRawData::JsRawData(const JsWeak& data) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	m_data = data.m_data;
}
kr::JsRawData::JsRawData(const JsRawDataValue& data) noexcept
{
	m_data = data;
}
kr::JsRawData::JsRawData(JsRawDataValue&& data) noexcept
{
	m_data = data;
}
kr::JsRawData::JsRawData(int value) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsIntToNumber(value, &m_data);
}
kr::JsRawData::JsRawData(double value) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsDoubleToNumber(value, &m_data);
}
kr::JsRawData::JsRawData(bool value) noexcept
{
	_assert(s_context->m_undefinedValue);
	m_data = value ? s_context->m_trueValue : s_context->m_falseValue;
}
kr::JsRawData::JsRawData(nullptr_t) noexcept
{
	_assert(s_context->m_undefinedValue);
	m_data = s_context->m_nullValue;
}
kr::JsRawData::JsRawData(undefined_t) noexcept
{
	_assert(s_context->m_undefinedValue);
	m_data = s_context->m_undefinedValue;
}
kr::JsRawData::JsRawData(JsNewObject_t) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsCreateObject(&m_data);
}
kr::JsRawData::JsRawData(JsNewSymbol symbol) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsCreateSymbol(symbol.description.getRaw(), &m_data);
}
kr::JsRawData::JsRawData(JsNewArray arr) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsCreateArray(intact<uint>(arr.size), &m_data);
}
kr::JsRawData::JsRawData(JsNewTypedArray arr) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsCreateTypedArray(InternalTools::typedArrayTypeToJsrt(arr.type), arr.arrayBuffer.m_data, 0, intact<unsigned int>(arr.size), &m_data);
}
kr::JsRawData::JsRawData(JsNewArrayBuffer arr) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	NOERR JsCreateArrayBuffer(intact<unsigned int>(arr.bytes), &m_data);
}

bool kr::JsRawData::isEmpty() const noexcept
{
	return m_data == JS_INVALID_REFERENCE;
}
kr::JsType kr::JsRawData::getType() const noexcept
{
	if (((uintptr_t)m_data >> 48) == 1)
	{
		return JsType::Integer;
	}

	JsValueType type;
	NOERR JsGetValueType(m_data, &type);
	switch (type)
	{
	case JsUndefined: return JsType::Undefined;
	case JsNull: return JsType::Null;
	case JsNumber: return JsType::Float;
	case ::JsString: return JsType::String;
	case JsBoolean: return JsType::Boolean;
	case ::JsObject: return JsType::Object;
	case ::JsFunction: return JsType::Function;
	case JsError:
	case JsArray:
	case ::JsSymbol:
		return JsType::Object;
	case JsArrayBuffer: return JsType::ArrayBuffer;
	case JsTypedArray: return JsType::TypedArray;
	case JsDataView: return JsType::DataView;
	default:
		unreachable();
	}
}
void kr::JsRawData::setByIndex(const JsRawData& name, const JsRawData& value) const noexcept
{
	NOERR JsSetIndexedProperty(m_data, name.m_data, value.m_data);
}
void kr::JsRawData::setByProperty(const JsPropertyId& name, const JsRawData& value) const noexcept
{
	NOERR JsSetProperty(m_data, name.m_data, value.m_data, true);
}
kr::JsRawData kr::JsRawData::getByIndex(const JsRawData& name) const noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	JsRawData out;
	NOERR JsGetIndexedProperty(m_data, name.m_data, &out.m_data);
	return out;
}
kr::JsRawData kr::JsRawData::getConstructor() const noexcept
{
	return getByProperty((JsPropertyId)s_context->m_constructorId);
}
void kr::JsRawData::freeze() noexcept
{
	NOERR JsPreventExtension(m_data);
}
bool kr::JsRawData::isFreezed() noexcept
{
	bool value;
	NOERR JsGetExtensionAllowed(m_data, &value);
	return !value;
}
kr::JsRawData kr::JsRawData::getByProperty(const JsPropertyId& name) const noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	JsRawData out;
	NOERR JsGetProperty(m_data, name.m_data, &out.m_data);
	return out;
}
bool kr::JsRawData::prototypeOf(const JsRawData& value) const noexcept
{

	JsValueRef proto = m_data;
	for (;;)
	{
		if (proto == value.m_data) return true;
		JsErrorCode err = JsGetPrototype(proto, &proto);
		if (err != JsNoError) return false;
	}
}
bool kr::JsRawData::instanceOf(const JsRawData& value) const noexcept
{	
	bool res;
	NOERR JsInstanceOf(m_data, value.m_data, &res);
	return res;
}
void* kr::JsRawData::getExternalData() const noexcept
{
	void* res;
	NOERR JsGetExternalData(m_data, &res);
	return res;
}
kr::WBuffer kr::JsRawData::getArrayBuffer() const noexcept
{
	_assert(getType() == JsType::ArrayBuffer);
	BYTE* out;
	unsigned int length;
	NOERR JsGetArrayBufferStorage(m_data, &out, &length);
	return WBuffer(out, length);
}
kr::WBuffer kr::JsRawData::getDataViewBuffer() const noexcept
{
	_assert(getType() == JsType::DataView);
	BYTE* out;
	unsigned int length;
	// JsGetTypedArrayInfo()
	NOERR JsGetDataViewStorage(m_data, &out, &length);
	return WBuffer(out, length);

}
kr::WBuffer kr::JsRawData::getTypedArrayBuffer(JsTypedType* type) const noexcept
{
	_assert(getType() == JsType::TypedArray);
	BYTE* out;
	unsigned int length;
	int elementLength;
	::JsTypedArrayType arrayType;
	NOERR JsGetTypedArrayStorage(m_data, &out, &length, &arrayType, &elementLength);
	*type = InternalTools::typedArrayTypeToKr(arrayType);
	return WBuffer(out, length);
}
int32_t kr::JsRawData::getArrayLength() const throws(JsException)
{
	JsValueRef jslength;
	NOERR JsGetProperty(m_data, s_context->m_lengthId, &jslength);
	int length;
	ERRCT JsNumberToInt(jslength, &length);
	if (length < 0)
	{
		TText16 text;
		text << u"length is " << length;
		throw JsException((Text16)text);
	}
	return length;
}
void kr::JsRawData::setArrayLength(size_t length) const throws(JsException)
{
	JsValueRef jslength;
	ERRCT JsIntToNumber(intact<int>(length), &jslength);
	NOERR JsSetProperty(m_data, s_context->m_lengthId, jslength, true);
}
kr::JsRawData kr::JsRawData::call(JsRawData _this, JsArgumentsIn arguments) const throws(JsException)
{
	ondebug(_assert(s_scopeStackCounter != 0));

	TmpArray<JsValueRef> args;
	args.push(_this.m_data);
	for (const JsValue& value : arguments)
	{
		args.push(value.m_data);
	}

	JsRawData res;
	ERRCT JsCallFunction(m_data, args.data(), intact<unsigned short>(args.size()), &res.m_data);
	return res;
}
bool kr::JsRawData::equals(const JsRawData& other) const noexcept
{
	bool res;
	NOERR JsStrictEquals(m_data, other.m_data, &res);
	return res;
}
bool kr::JsRawData::abstractEquals(const JsRawData& other) const noexcept
{
	bool res;
	NOERR JsEquals(m_data, other.m_data, &res);
	return res;
}
kr::JsRawData kr::JsRawData::toString() const throws(JsException)
{
	ondebug(_assert(s_scopeStackCounter != 0));

	JsRawData value;
	NOERR JsConvertValueToString(m_data, &value.m_data);
	return value;
}
kr::JsRawData kr::JsRawData::valueOf() const throws(JsException)
{
	ondebug(_assert(s_scopeStackCounter != 0));

	JsRawData value;
	NOERR JsConvertValueToNumber(m_data, &value.m_data);
	return value;
}

template <>
kr::Text16 kr::JsRawData::as<kr::Text16>() const noexcept
{
	_assert(getType() == JsType::String);
	const wchar_t* out;
	size_t sz;
	NOERR JsStringToPointer(m_data, &out, &sz);
	return Text16(unwide(out), sz);
}
template <>
kr::JsObject* kr::JsRawData::as<kr::JsObject*>() const noexcept
{
	_assert(instanceOf(*s_nativeClass));
	if (!InternalTools::hasExternalData(m_data)) return nullptr;
	return (JsObject*)getExternalData();
}
template <>
int kr::JsRawData::as<int>() const noexcept
{
	int out;
	NOERR JsNumberToInt(m_data, &out);
	return out;
}
template <>
double kr::JsRawData::as<double>() const noexcept
{
	double out;
	NOERR JsNumberToDouble(m_data, &out);
	return out;
}
template <>
bool kr::JsRawData::as<bool>() const noexcept
{
	bool out;
	NOERR JsBooleanToBool(m_data, &out);
	return out;
}

// #define REF_VIEW

void logRef(JsValueRef value, bool release) noexcept
{
#ifdef REF_VIEW
	using namespace kr;
	if (value == JS_INVALID_REFERENCE) return;
	cout << value;
	if (release)
	{
		cout << ": release ";
	}
	else
	{
		cout << ": addref ";
	}
	cout << InternalTools::getRef(value) << endl;
#endif
}

// persistent
kr::JsPersistent::JsPersistent() noexcept
{
	m_data = JS_INVALID_REFERENCE;
}
kr::JsPersistent::JsPersistent(const JsRawData& value) noexcept
{
	m_data = value.m_data;
	JsAssertAddRef(m_data);
	logRef(m_data, false);
}
kr::JsPersistent::JsPersistent(const JsWeak& obj) noexcept
{
	m_data = obj.m_data;
	NOERR JsAddRef(m_data, nullptr);
	logRef(m_data, false);
}
kr::JsPersistent::JsPersistent(const JsPersistent& obj) noexcept
{
	m_data = obj.m_data;
	NOERR JsAddRef(m_data, nullptr);
	logRef(m_data, false);
}
kr::JsPersistent::JsPersistent(JsPersistent&& obj) noexcept
{
	m_data = obj.m_data;
	obj.m_data = JS_INVALID_REFERENCE;
}
kr::JsPersistent::~JsPersistent() noexcept
{
	logRef(m_data, true);
	JsAssertRelease(m_data);
}
bool kr::JsPersistent::isEmpty() const noexcept
{
	return m_data == JS_INVALID_REFERENCE;
}

// weak
kr::JsWeak::JsWeak() noexcept
{
	m_data = JS_INVALID_REFERENCE;
}
kr::JsWeak::JsWeak(const JsRawData& value) noexcept
{
	m_data = value.m_data;
}
kr::JsWeak::JsWeak(const JsPersistent& obj) noexcept
{
	m_data = obj.m_data;
}
kr::JsWeak::JsWeak(const JsWeak& obj) noexcept
{
	m_data = obj.m_data;
}
kr::JsWeak::JsWeak(JsWeak&& obj) noexcept
{
	m_data = obj.m_data;
}
kr::JsWeak::~JsWeak() noexcept
{
}
bool kr::JsWeak::isEmpty() const noexcept
{
	return m_data == JS_INVALID_REFERENCE;
}

// arguments
kr::JsArgumentsAllocated::JsArgumentsAllocated(const JsValue& _this, size_t argn) noexcept
	:JsArguments(_this, _new JsValue[argn], argn)
{
}
kr::JsArgumentsAllocated::~JsArgumentsAllocated() noexcept
{
	delete[] m_args;
}

// exception
kr::JsException::JsException() noexcept
{
	m_exception = JS_INVALID_REFERENCE;
}
kr::JsException::JsException(kr::Text16 message) noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));
	m_exception = InternalTools::createError(message).m_data;
}
kr::JsException::JsException(JsException&& _move) noexcept
{
	m_exception = _move.m_exception;
	_move.m_exception = JS_INVALID_REFERENCE;
}
kr::Text16 kr::JsException::toString() const noexcept
{
	try
	{
		return JsRawData(m_exception).toString().as<Text16>();
	}
	catch (JsException&)
	{
		return u"Error.toString failed";
	}
}
kr::JsValue kr::JsException::getValue() const noexcept
{
	return JsRawData(m_exception);
}
bool kr::JsException::isEmpty() const noexcept
{
	return m_exception == JS_INVALID_REFERENCE;
}

// object
kr::JsObjectT<kr::JsObject>::JsObjectT(const JsArguments& args) throws(JsObjectT)
	:JsWeak(args.getThis())
{
}
kr::JsObjectT<kr::JsObject>::~JsObjectT() noexcept
{
	JsSetExternalData(m_data, nullptr);
}

// property id
kr::JsPropertyId::JsPropertyId() noexcept
	:m_data(JS_INVALID_REFERENCE)
{
}
kr::JsPropertyId::JsPropertyId(pcstr16 name) noexcept
{
	NOERR JsGetPropertyIdFromName(wide(name), &m_data);
	NOERR JsAddRef(m_data, nullptr);
}
kr::JsPropertyId::JsPropertyId(const JsPropertyId& name) noexcept
	:m_data(name.m_data)
{
	if (m_data == JS_INVALID_REFERENCE) return;
	NOERR JsAddRef(m_data, nullptr);
}
kr::JsPropertyId::JsPropertyId(JsPropertyId&& name) noexcept
	:m_data(move(name.m_data))
{
	name.m_data = JS_INVALID_REFERENCE;
}
kr::JsPropertyId::JsPropertyId(const JsRawPropertyId& name) noexcept
	:m_data(name)
{
}
kr::JsPropertyId::JsPropertyId(JsRawPropertyId&& name) noexcept
	:m_data(move(name))
{
}
kr::JsPropertyId::~JsPropertyId() noexcept
{
	JsAssertRelease(m_data);
}
kr::JsPropertyId kr::JsPropertyId::fromSymbol(JsRawData value) noexcept
{
	JsPropertyIdRef prop;
	JsGetPropertyIdFromSymbol(value.getRaw(), &prop);
	return kr::JsPropertyId(prop);
}


// native function
kr::JsValue kr::JsFunction::Data::create() noexcept
{
	return InternalTools::createExternalFunction(InternalTools::nativeFunctionCallback, this);
}

// class info
kr::_pri_::JsClassInfo::JsClassInfo(Text16 name, size_t parentIdx, CTOR ctor, ClearMethods(*initMethods)(), bool global) noexcept
{
	static size_t s_indexCounter = 0;
	
	m_index = s_indexCounter++;
	m_name = name;
	m_parentIndex = parentIdx;
	m_ctor = ctor;
	m_initMethods = initMethods;
	m_isGlobal = global;
#ifndef NDEBUG
	static Set<JsClassInfo*> inited;
	_assert(inited.insert(this).second == true);
	m_inited = false;
#endif
	_assert(m_index != 0 || this == &JsObject::s_classInfo);

	JsRuntime::initpack.add<JsClassInfo>([](JsClassInfo* _this){
#ifndef NDEBUG
		_assert(!_this->m_inited);
		_this->m_inited = true;
#endif

		JsRawData name(_this->m_name);
		JsRawData cls = InternalTools::createClass(name, _this->m_ctor, s_context->m_prototypeId, s_context->m_constructorId);
		JsAddRef(cls.getRaw(), nullptr);

		*s_context->m_classes.prepare(1) = cls;
		*_this->get() = cls;

		if (_this->m_isGlobal)
		{
			NOERR JsSetIndexedProperty(s_context->m_global, name.getRaw(), cls.getRaw());
		}

		if (_this->m_parentIndex != -1) InternalTools::extends(cls, s_context->m_classes[_this->m_parentIndex], s_context->m_prototypeId);
		return _this->m_initMethods();
		}, this);
}

// class
kr::JsClass kr::JsClass::_createChild(Text16 _name, CTOR ctor) const noexcept
{
	ondebug(_assert(s_scopeStackCounter != 0));

	JsRawData name(_name);
	JsRawData cls = InternalTools::createClass(name,  ctor, s_context->m_prototypeId, s_context->m_constructorId);
	InternalTools::extends(cls, *this, s_context->m_prototypeId);
	return cls;
}
void kr::JsClass::setField(const JsValue &name, const JsValue &_v) throws(JsException)
{
	JsScope scope;
	JsValue prototype = get((JsPropertyId)s_context->m_prototypeId);
	prototype.set(name, _v);
}
void kr::JsClass::setField(const JsPropertyId &name, const JsValue& _v) throws(JsException)
{
	JsScope scope;
	JsValue prototype = get((JsPropertyId)s_context->m_prototypeId);
	prototype.set(name, _v);
}
void kr::JsClass::setAccessorRaw(const JsPropertyId& name, JsAccessor* _accessor) noexcept
{
	JsValue prototype = get((JsPropertyId)s_context->m_prototypeId);
	JsValue descriptor = JsNewObject;
	JsValue getter = InternalTools::createExternalFunction(InternalTools::getterCallback, _accessor);
	JsValue setter = InternalTools::createExternalFunction(InternalTools::setterCallback, _accessor);
	
	NOERR JsSetProperty(descriptor.m_data, s_context->m_getId, getter.m_data, true);
	NOERR JsSetProperty(descriptor.m_data, s_context->m_setId, setter.m_data, true);

	bool res;
	NOERR JsDefineProperty(prototype.m_data, name.m_data, descriptor.m_data, &res);
	_assert(res);
}
void kr::JsClass::setGetterRaw(const JsPropertyId &name, JsGetter* _accessor) noexcept
{
	JsValue prototype = get((JsPropertyId)s_context->m_prototypeId);
	JsValue descriptor = JsNewObject;
	JsValue getter = InternalTools::createExternalFunction(InternalTools::getterCallback, _accessor);
	NOERR JsSetProperty(descriptor.m_data, s_context->m_getId, getter.m_data, true);

	bool res;
	NOERR JsDefineProperty(prototype.m_data, name.m_data, descriptor.m_data, &res);
	_assert(res);
}
void kr::JsClass::setIndexAccessorRaw(JsIndexAccessor* _accessor) noexcept
{
	debug(); // TODO: implement
	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();

	//prototype->SetIndexedPropertyHandler(
	//	[](uint32_t idx, const PropertyCallbackInfo<Value>& _info) {
	//		JsIndexAccessor* d = getExternal<JsIndexAccessor>(_info.Data());
	//		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()), idx)));
	//	},
	//	[](uint32_t idx, Local<Value> _value, const PropertyCallbackInfo<Value>& _info) {
	//		JsIndexAccessor* d = getExternal<JsIndexAccessor>(_info.Data());
	//		d->set(Tools::toJS(_info.This()), idx, Tools::toJSValue(_value));
	//	}, nullptr, nullptr, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external));
	_accessor->Release();
}
void kr::JsClass::setReadOnlyIndexAccessorRaw(JsIndexAccessor* _accessor) noexcept
{
	debug(); // TODO: implement
	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();

	//prototype->SetIndexedPropertyHandler(
	//	[](uint32_t idx, const PropertyCallbackInfo<Value>& _info) {
	//		JsIndexAccessor* d = getExternal<JsIndexAccessor>(_info.Data());
	//		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()), idx)));
	//	}, nullptr, nullptr, nullptr, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external));
	_accessor->Release();
}
void kr::JsClass::setStaticAccessorRaw(const JsPropertyId& name, JsAccessor* _accessor) noexcept
{
	JsValue descriptor = JsNewObject;
	JsValue getter = InternalTools::createExternalFunction(InternalTools::getterCallback, _accessor);
	JsValue setter = InternalTools::createExternalFunction(InternalTools::setterCallback, _accessor);

	NOERR JsSetProperty(descriptor.m_data, s_context->m_getId, getter.m_data, true);
	NOERR JsSetProperty(descriptor.m_data, s_context->m_setId, setter.m_data, true);

	bool res;
	NOERR JsDefineProperty(m_data, name.m_data, descriptor.m_data, &res);
	_assert(res);
}
void kr::JsClass::setStaticGetterRaw(const JsPropertyId& name, JsGetter* _accessor) noexcept
{
	JsValue descriptor = JsNewObject;
	JsValue getter = InternalTools::createExternalFunction(InternalTools::getterCallback, _accessor);
	NOERR JsSetProperty(descriptor.m_data, s_context->m_getId, getter.m_data, true);

	bool res;
	NOERR JsDefineProperty(m_data, name.m_data, descriptor.m_data, &res);
	_assert(res);
}
kr::JsValue kr::JsClass::newInstanceRaw(JsArgumentsIn args) const throws(JsException)
{
	ondebug(_assert(s_scopeStackCounter != 0));

	JsRawData instance;
	{
		JsScope scope;
		TmpArray<JsValueRef> nargs;
		nargs.push(s_context->m_undefinedValue);
		for (const JsValue& arg : args)
		{
			nargs.push(arg.m_data);
		}
		ERRCT JsConstructObject(m_data, nargs.data(), intact<unsigned short>(nargs.size()), &instance.m_data);
	}
	return (JsRawData)instance;
}

// runtime
kr::JsRuntime::JsRuntime() noexcept
{
	_assert(!s_runtime);
	JsRuntimeRef runtime;
	NOERR JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);
	setRuntime(runtime);

}
kr::JsRuntime::~JsRuntime() noexcept
{
	dispose();
}

kr::JsValue kr::JsRuntime::run(Text16 fileName, Text16 source) throws(JsException)
{
	ondebug(_assert(s_scopeStackCounter != 0));

	JsRawData result;
	TText16 buf2;
	TText16 buf;
	LPCWSTR source_sz = szlize(source, &buf2);
	LPCWSTR fileName_sz = szlize(fileName, &buf);
	ERRCT JsRunScript(
		source_sz,
		s_sourceContextCounter++, fileName_sz,
		&result.m_data);
	return result;
}
kr::JsValue kr::JsRuntime::run(Text16 fileName, Text16 source, uintptr_t sourceContext) throws(JsException)
{
	ondebug(_assert(s_scopeStackCounter != 0));

	JsRawData result;
	TText16 buf2;
	TText16 buf;
	LPCWSTR source_sz = szlize(source, &buf2);
	LPCWSTR fileName_sz = szlize(fileName, &buf);
	ERRCT JsRunScript(source_sz, sourceContext, fileName_sz, &result.m_data);
	return result;
}
kr::JsValue kr::JsRuntime::global() noexcept
{
	return (JsRawData)s_context->m_global;
}
const kr::JsRawRuntime& kr::JsRuntime::getRaw() noexcept
{
	return s_runtime;
}
void kr::JsRuntime::setRuntime(const JsRawRuntime &runtime) noexcept
{
	s_runtime = runtime;
	s_nativeClass = &JsObject::classObject;
}
void kr::JsRuntime::dispose() noexcept
{
	JsDisposeRuntime(s_runtime);
	s_runtime = JS_INVALID_REFERENCE;
	s_sourceContextCounter = 0;
}
void kr::JsRuntime::gc() noexcept
{
	NOERR JsCollectGarbage(s_runtime);
}
void kr::JsRuntime::idle() noexcept
{
	NOERR JsIdle(nullptr);
}
void kr::JsRuntime::test() noexcept
{
	return InternalTools::test();
}

// context
kr::JsContext::JsContext() noexcept
	:JsContext(InternalTools::createContext())
{
	JsSetPromiseContinuationCallback([](JsValueRef value, void* state) {
		JsScope scope;
		JsRawData(value).call((JsRawData)undefined, {});
		}, nullptr);
}
kr::JsContext::JsContext(const JsContext& _ctx) noexcept
{
	JsAddRef(m_context = _ctx.m_context, nullptr);
	JsAddRef(m_global = _ctx.m_global, nullptr);
	JsAddRef(m_prototypeId = _ctx.m_prototypeId, nullptr);
	JsAddRef(m_constructorId = _ctx.m_constructorId, nullptr);
	JsAddRef(m_lengthId = _ctx.m_lengthId, nullptr);
	JsAddRef(m_getId = _ctx.m_getId, nullptr);
	JsAddRef(m_setId = _ctx.m_setId, nullptr);
	
	m_classes = _ctx.m_classes;
	JsRawData* clsptr = m_classes.data();
	for (JsRawData cls : m_classes)
	{
		JsAddRef(cls.m_data, nullptr);
	}
}
kr::JsContext::JsContext(const JsRawContext& ctx) noexcept
{
	m_context = ctx;
	JsAddRef(ctx, nullptr);
	JsContextRef oldctx = InternalTools::getCurrentContext();
	JsSetCurrentContext(m_context);
	NOERR JsGetGlobalObject(&m_global);
	NOERR JsAddRef(m_global, nullptr);
	NOERR JsGetUndefinedValue(&m_undefinedValue);
	NOERR JsGetTrueValue(&m_trueValue);
	NOERR JsGetFalseValue(&m_falseValue);
	NOERR JsGetNullValue(&m_nullValue);
	NOERR JsGetPropertyIdFromName(L"prototype", &m_prototypeId);
	JsAddRef(m_prototypeId, nullptr);
	NOERR JsGetPropertyIdFromName(L"constructor", &m_constructorId);
	JsAddRef(m_constructorId, nullptr);
	NOERR JsGetPropertyIdFromName(L"length", &m_lengthId);
	JsAddRef(m_lengthId, nullptr);
	NOERR JsGetPropertyIdFromName(L"get", &m_getId);
	JsAddRef(m_getId, nullptr);
	NOERR JsGetPropertyIdFromName(L"set", &m_setId);
	JsAddRef(m_setId, nullptr);
	s_context = this;

	{
		JsScope scope;
		JsRuntime::initpack.init();
	}
	JsSetCurrentContext(oldctx);
}
kr::JsContext::~JsContext() noexcept
{
	JsContextRef oldctx = InternalTools::getCurrentContext();
	JsSetCurrentContext(m_context);
	{
		JsScope scope;
		JsRuntime::initpack.clear();
	}
	for (JsRawData value : m_classes.reverse())
	{
		JsAssertRelease(value.m_data);
	}
	JsAssertRelease(m_prototypeId);
	JsAssertRelease(m_constructorId);
	JsAssertRelease(m_lengthId);
	JsAssertRelease(m_getId);
	JsAssertRelease(m_setId);
	JsAssertRelease(m_global);
	JsSetCurrentContext(oldctx);
	JsAssertRelease(m_context);
}

const kr::JsRawContext& kr::JsContext::getRaw() noexcept
{
	return m_context;
}
void kr::JsContext::enter() noexcept
{
	ondebug(_assert(s_scopeStackCounter == 0));
	s_context = this;
	JsSetCurrentContext(m_context);
	ondebug(s_scopeStackCounter++);
}

#pragma warning(push)
#pragma warning(disable:6387)
void kr::JsContext::exit() noexcept
{
	_assert(InternalTools::getCurrentContext() == m_context);
	ondebug(s_scopeStackCounter--);
	JsSetCurrentContext(nullptr);
}
void kr::JsContext::exitCurrent() noexcept
{
	ondebug(s_scopeStackCounter--);
	JsSetCurrentContext(nullptr);
}
void kr::JsContext::_exit() noexcept
{
	JsSetCurrentContext(nullptr);
}
#pragma warning(pop)

void kr::JsContext::_cleanStackCounter() noexcept
{
	ondebug(s_scopeStackCounter = 0);
}

#else

EMPTY_SOURCE

#endif