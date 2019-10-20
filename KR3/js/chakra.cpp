#include "stdafx.h"

#ifndef KRJS_USE_V8

#include "js.h"

#include <KR3/util/wide.h>

#define USE_EDGEMODE_JSRT
#include <jsrt.h>
#pragma comment(lib, "chakrart.lib")

#pragma warning(disable:4073)
#pragma init_seg(lib)


using kr::_pri_::JsClassList;
using kr::_pri_::InternalTools;

class kr::_pri_::JsClassList
{
public:
	JsClassInfo* _begin;
	JsClassInfo** _end;

	struct Iterator
	{
		JsClassInfo* pointer;
		JsClassInfo& operator *() const noexcept
		{
			return *pointer;
		}
		JsClassInfo* operator ->() const noexcept
		{
			return pointer;
		}
		bool operator !=(const IteratorEnd&) const noexcept
		{
			return pointer != nullptr;
		}
		Iterator& operator ++() noexcept
		{
			pointer = pointer->next();
			return *this;
		}
	};

	JsClassList() noexcept
	{
		_begin = nullptr;
		_end = &_begin;
	}
	void add(JsClassInfo* cls) noexcept
	{
		*_end = cls;
		_end = &cls->m_next;
	}
	Iterator begin() noexcept
	{
		Iterator iter;
		iter.pointer = _begin;
		return iter;
	}
	kr::IteratorEnd end()
	{
		return kr::IteratorEnd();
	}
};

namespace
{	
	JsValueRef s_undefinedValue;
	JsValueRef s_trueValue;
	JsValueRef s_falseValue;
	JsValueRef s_nullValue;
	JsValueRef s_globalObject;
	JsPropertyIdRef s_prototypeId;
	JsPropertyIdRef s_constructorId;
	JsClassList s_classList;

	kr::JsClassT<kr::JsObject> * s_nativeClass;
	JsRuntimeHandle s_runtime;
	JsSourceContext s_sourceContextCounter;
	kr::JsScope* s_currentScope;
	kr::Manual<kr::JsScope> s_manualScope;
}

#define jsassert(...) do { JsErrorCode err = __VA_ARGS__; _assert(err == JsNoError); } while(0, 0)
#define jsthrow(...) do { JsErrorCode err = __VA_ARGS__; \
	if (err == JsErrorScriptException) {\
		JsValueRef exception; \
		jsassert(JsGetAndClearException(&exception));\
		throw JsException(exception);\
	} \
	_assert(err == JsNoError);\
} while(0, 0)

namespace kr
{
	namespace _pri_
	{
		struct InternalTools
		{
			static const JsRawData& detachValue(JsScope& scope, const JsRawData& ref) noexcept
			{
				size_t idx = scope.m_refs.pos(ref.m_data);
				if (idx == -1)
				{
					// for global registered values
					return ref;
				}
				scope.m_refs.pickGet(idx);
				return ref;
			}
			static JsTypedArrayType typedArrayTypeToKr(::JsTypedArrayType type) noexcept
			{
				static_assert(
					(int)JsTypedArrayType::Int8 == JsArrayTypeInt8 &&
					(int)JsTypedArrayType::Uint8 == JsArrayTypeUint8 &&
					(int)JsTypedArrayType::Uint8Clamped == JsArrayTypeUint8Clamped &&
					(int)JsTypedArrayType::Int16 == JsArrayTypeInt16 &&
					(int)JsTypedArrayType::Uint16 == JsArrayTypeUint16 &&
					(int)JsTypedArrayType::Int32 == JsArrayTypeInt32 &&
					(int)JsTypedArrayType::Uint32 == JsArrayTypeUint32 &&
					(int)JsTypedArrayType::Float32 == JsArrayTypeFloat32 &&
					(int)JsTypedArrayType::Float64 == JsArrayTypeFloat64, "Enum not match");
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
				return (JsTypedArrayType)type;
			}
			static ::JsTypedArrayType typedArrayTypeToJsrt(JsTypedArrayType type) noexcept
			{
				static_assert(
					(int)JsTypedArrayType::Int8 == JsArrayTypeInt8 &&
					(int)JsTypedArrayType::Uint8 == JsArrayTypeUint8 &&
					(int)JsTypedArrayType::Uint8Clamped == JsArrayTypeUint8Clamped &&
					(int)JsTypedArrayType::Int16 == JsArrayTypeInt16 &&
					(int)JsTypedArrayType::Uint16 == JsArrayTypeUint16 &&
					(int)JsTypedArrayType::Int32 == JsArrayTypeInt32 &&
					(int)JsTypedArrayType::Uint32 == JsArrayTypeUint32 &&
					(int)JsTypedArrayType::Float32 == JsArrayTypeFloat32 &&
					(int)JsTypedArrayType::Float64 == JsArrayTypeFloat64, "Enum not match");
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
				JsRawData out;
				jsassert(JsCreateError(JsRawData(message).m_data, &out.m_data));
				s_currentScope->add(out);
				return out;
			}
			static JsArguments makeArgs(JsRawData callee, JsRawData _this, JsValueRef* arguments, unsigned short argumentCount) noexcept
			{
				arguments++;

				JsArguments args(callee, _this, argumentCount - 1);
				for (JsValue& v : args)
				{
					JsValueRef ref = *arguments++;
					v.m_data = ref;
				}
				return args;
			}
			static JsValueRef CT_STDCALL nativeConstructorCallback(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState) noexcept
			{
				if (!isConstructCall)
				{
					jsassert(JsSetException(createError(u"Needs new operator").m_data));
					return JS_INVALID_REFERENCE;
				}
				JsScope scope;
				
				JsRawData _thisref;
				jsassert(JsCreateExternalObject(nullptr, [](void * data) {
					if (data == nullptr) return;
					((JsObject*)data)->finallize();
				}, & _thisref.m_data));

				JsRawData prototype;
				JsGetProperty(callee, s_prototypeId, &prototype.m_data);
				JsSetPrototype(_thisref.m_data, prototype.m_data);
				scope.add(prototype);

				JsArguments args = makeArgs((JsRawData)callee, _thisref, arguments, argumentCount);

				using CTOR = JsClass::CTOR;
				CTOR ctor = (CTOR)callbackState;
				try
				{
					JsObject* jsobj = ctor(args);
					JsSetExternalData(_thisref.m_data, jsobj);
					return _thisref.m_data;
				}
				catch (JsException & e)
				{
					jsassert(JsSetException(e.m_exception));
					scope.add(_thisref);
					return JS_INVALID_REFERENCE;
				}
			}
			static JsValueRef CT_STDCALL nativeFunctionCallback(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState) noexcept
			{
				kr::JsFunction::Data*  data = (kr::JsFunction::Data*)callbackState;
				try
				{
					JsScope scope;
					JsArguments args = makeArgs((JsRawData)callee, (JsRawData)*arguments, arguments, argumentCount);
					JsValue retValue = data->call(args);
					return detachValue(scope, retValue).m_data;
				}
				catch (JsException& e)
				{
					JsSetException(e.m_exception);
					return JS_INVALID_REFERENCE;
				}
			}

			static JsContextRef createContext() noexcept
			{
				JsContextRef ctx;
				jsassert(JsCreateContext(s_runtime, &ctx));
				return ctx;
			}
			static JsContextRef getCurrentContext() noexcept
			{
				JsContextRef oldctx;
				jsassert(JsGetCurrentContext(&oldctx));
				return oldctx;
			}
			static bool checkCurrentContext(JsContextRef ctx) noexcept
			{
				JsContextRef oldctx;
				jsassert(JsGetCurrentContext(&oldctx));
				bool res = oldctx == ctx;
				JsRelease(oldctx, nullptr);
				return res;
			}
			static bool hasExternalData(JsValueRef object) noexcept
			{
				bool out;
				jsassert(JsHasExternalData(object, &out));
				return out;
			}
			
			static JsRawData createClass(JsRawData name, JsClass::CTOR ctor, JsPropertyIdRef prototypeId) noexcept
			{
				JsRawData cls;
				jsassert(JsCreateNamedFunction(name.m_data, InternalTools::nativeConstructorCallback, ctor, &cls.m_data));
				s_currentScope->add(cls);

				JsRawData prototype;
				jsassert(JsCreateObject(&prototype.m_data));
				s_currentScope->add(prototype);
				jsassert(JsSetProperty(cls.m_data, prototypeId, prototype.m_data, true));
				return cls;
			}
			static void extends(JsRawData child, JsRawData parent, JsPropertyIdRef prototypeId, JsPropertyIdRef constructorId) noexcept
			{
				JsRawData prototype;
				jsassert(JsGetProperty(parent.m_data, prototypeId, &prototype.m_data));
				s_currentScope->add(prototype);
				jsassert(JsSetProperty(child.m_data, prototypeId, prototype.m_data, true));
				jsassert(JsSetProperty(child.m_data, constructorId, parent.m_data, true));
			}
		};
	}
}

// scope
kr::JsScope::JsScope() noexcept
{
	m_prev = s_currentScope;
	s_currentScope = this;
}
kr::JsScope::~JsScope() noexcept
{
	_assert(s_currentScope == this);
	for (JsValueRef ref : m_refs)
	{
		JsRelease(ref, nullptr);
	}
	s_currentScope = m_prev;
}
void kr::JsScope::add(const JsRawData& ref) noexcept
{
	_assert(this != nullptr);
	m_refs.push(ref.m_data);
}
const kr::JsRawData& kr::JsScope::returnValue(const JsRawData &value) noexcept
{
	InternalTools::detachValue(*this, value);
	m_prev->add(value);
	return value;
}

// rawdata
kr::JsRawData::JsRawData() noexcept
{
	m_data = JS_INVALID_REFERENCE;
}
kr::JsRawData::JsRawData(Text16 text) noexcept
{
	jsassert(JsPointerToString(wide(text.data()), text.size(), &m_data));
	s_currentScope->add(*this);
}
kr::JsRawData::JsRawData(const JsPersistent& data) noexcept
{
	m_data = data.m_data;
	s_currentScope->add(*this);
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
	jsassert(JsIntToNumber(value, &m_data));
	s_currentScope->add(*this);
}
kr::JsRawData::JsRawData(double value) noexcept
{
	jsassert(JsDoubleToNumber(value, &m_data));
	s_currentScope->add(*this);
}
kr::JsRawData::JsRawData(bool value) noexcept
{
	_assert(s_undefinedValue);
	m_data = value ? s_trueValue : s_falseValue;
}
kr::JsRawData::JsRawData(nullptr_t) noexcept
{
	_assert(s_undefinedValue);
	m_data = s_nullValue;
}
kr::JsRawData::JsRawData(undefined_t) noexcept
{
	_assert(s_undefinedValue);
	m_data = s_undefinedValue;
}
kr::JsRawData::JsRawData(JsNewObject_t) noexcept
{
	jsassert(JsCreateObject(&m_data));
	s_currentScope->add(*this);
}
kr::JsRawData::JsRawData(JsNewArray arr) noexcept
{
	jsassert(JsCreateArray(intact<uint>(arr.size), &m_data));
	s_currentScope->add(*this);
}
kr::JsRawData::JsRawData(JsNewTypedArray arr) noexcept
{
	jsassert(JsCreateTypedArray(InternalTools::typedArrayTypeToJsrt(arr.type), arr.arrayBuffer.m_data, 0, intact<unsigned int>(arr.size), &m_data));
	s_currentScope->add(*this);
}
kr::JsRawData::JsRawData(JsNewArrayBuffer arr) noexcept
{
	jsassert(JsCreateArrayBuffer(intact<unsigned int>(arr.bytes), &m_data));
	s_currentScope->add(*this);
}

bool kr::JsRawData::isEmpty() const noexcept
{
	return m_data == JS_INVALID_REFERENCE;
}
kr::JsType kr::JsRawData::getType() const noexcept
{
	JsValueType type;
	jsassert(JsGetValueType(m_data, &type));
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
	case JsSymbol:
		return JsType::Object;
	case JsArrayBuffer: return JsType::ArrayBuffer;
	case JsTypedArray: return JsType::TypedArray;
	case JsDataView: return JsType::DataView;
	default:
		unreachable();
	}
}
void kr::JsRawData::setProperty(Text16 name, const JsRawData& value) const noexcept
{
	jsassert(JsSetIndexedProperty(m_data, JsRawData(name).m_data, value.m_data));
}
void kr::JsRawData::setProperty(const JsPropertyId& name, const JsRawData& value) const noexcept
{
	jsassert(JsSetProperty(m_data, name.m_data, value.m_data, true));
}
kr::JsRawData kr::JsRawData::getProperty(Text16 name) const noexcept
{
	JsRawData out;
	jsassert(JsGetIndexedProperty(m_data, JsRawData(name).m_data, &out.m_data));
	s_currentScope->add(out);
	return out;
}
kr::JsRawData kr::JsRawData::getProperty(const JsPropertyId& name) const noexcept
{
	JsRawData out;
	jsassert(JsGetProperty(m_data, name.m_data, &out.m_data));
	s_currentScope->add(out);
	return out;
}
bool kr::JsRawData::instanceOf(const JsRawData& value) const noexcept
{
	bool res;
	jsassert(JsInstanceOf(m_data, value.m_data, &res));
	return res;
}
void* kr::JsRawData::getExternalData() const noexcept
{
	void* res;
	jsassert(JsGetExternalData(m_data, &res));
	return res;
}
kr::WBuffer kr::JsRawData::getArrayBuffer() const noexcept
{
	_assert(getType() == JsType::ArrayBuffer);
	BYTE* out;
	unsigned int length;
	jsassert(JsGetArrayBufferStorage(m_data, &out, &length));
	return WBuffer(out, length);
}
kr::WBuffer kr::JsRawData::getDataViewBuffer() const noexcept
{
	_assert(getType() == JsType::DataView);
	BYTE* out;
	unsigned int length;
	// JsGetTypedArrayInfo()
	jsassert(JsGetDataViewStorage(m_data, &out, &length));
	return WBuffer(out, length);

}
kr::WBuffer kr::JsRawData::getTypedArrayBuffer(JsTypedArrayType* type) const noexcept
{
	_assert(getType() == JsType::TypedArray);
	BYTE* out;
	unsigned int length;
	int elementLength;
	::JsTypedArrayType arrayType;
	jsassert(JsGetTypedArrayStorage(m_data, &out, &length, &arrayType, &elementLength));
	*type = InternalTools::typedArrayTypeToKr(arrayType);
	return WBuffer(out, length);
}
kr::JsRawData kr::JsRawData::call(JsRawData _this, JsArgumentsIn arguments) const throws(JsException)
{
	TmpArray<JsValueRef> args;
	args.push(_this.m_data);
	for (const JsValue& value : arguments)
	{
		args.push(value.m_data);
	}

	JsRawData res;
	jsthrow(JsCallFunction(m_data, args.data(), intact<unsigned short>(args.size()), &res.m_data));
	s_currentScope->add(res);
	return res;
}


template <>
kr::Text16 kr::JsRawData::as<kr::Text16>() const noexcept
{
	_assert(getType() == JsType::String);
	const wchar_t* out;
	size_t sz;
	jsassert(JsStringToPointer(m_data, &out, &sz));
	return Text16(unwide(out), sz);
}
template <>
kr::JsObject* kr::JsRawData::as<kr::JsObject*>() const noexcept
{
	_assert(InternalTools::hasExternalData(m_data));
	_assert(instanceOf(*s_nativeClass));
	return (JsObject*)getExternalData();
}
template <>
int kr::JsRawData::as<int>() const noexcept
{
	int out;
	jsassert(JsNumberToInt(m_data, &out));
	return out;
}
template <>
double kr::JsRawData::as<double>() const noexcept
{
	double out;
	jsassert(JsNumberToDouble(m_data, &out));
	return out;
}
template <>
bool kr::JsRawData::as<bool>() const noexcept
{
	bool out;
	jsassert(JsBooleanToBool(m_data, &out));
	return out;
}

// persistent
kr::JsPersistent::JsPersistent() noexcept
{
	m_data = JS_INVALID_REFERENCE;
}
kr::JsPersistent::JsPersistent(const JsRawData& value) noexcept
{
	m_data = value.m_data;
	JsAddRef(m_data, nullptr);
}
kr::JsPersistent::JsPersistent(const JsValue& value) noexcept
{
	m_data = value.m_data;
	JsAddRef(m_data, nullptr);
}
kr::JsPersistent::JsPersistent(const JsPersistent& obj) noexcept
{
	m_data = obj.m_data;
	JsAddRef(m_data, nullptr);
}
kr::JsPersistent::JsPersistent(JsPersistent&& obj) noexcept
{
	m_data = obj.m_data;
	obj.m_data = JS_INVALID_REFERENCE;
}
kr::JsPersistent::~JsPersistent() noexcept
{
	JsRelease(m_data, nullptr);
}
bool kr::JsPersistent::isEmpty() const noexcept
{
	return m_data == JS_INVALID_REFERENCE;
}

// exception
kr::JsException::JsException(JsRawException message) noexcept
{
	m_exception = message;
}
kr::JsException::JsException(kr::Text16 message) noexcept
{
	m_exception = InternalTools::createError(message).m_data;
}
kr::Text16 kr::JsException::toString() noexcept
{
	JsRawData value;
	jsassert(JsConvertValueToString(m_exception, &value.m_data));
	s_currentScope->add(value);
	return value.as<Text16>();
}

// property id
kr::JsPropertyId::JsPropertyId(pcstr16 name) noexcept
{
	jsassert(JsGetPropertyIdFromName(wide(name), &m_data));
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
	JsRelease(m_data, nullptr);
}

// native function
kr::JsValue kr::JsFunction::Data::create() noexcept
{
	JsRawData func;
	AddRef();
	jsassert(JsCreateFunction(InternalTools::nativeFunctionCallback, this, &func.m_data));
	JsSetObjectBeforeCollectCallback(func.m_data, this, [](JsRef func, void* state) {
		((Data*)state)->Release();
	});
	s_currentScope->add(func);
	return func;
}

// external
kr::JsExternal::JsExternal() noexcept
{
	m_reference = 0;
	jsassert(JsCreateExternalObject(this, [](void* data){
		((JsExternal*)data)->remove();
	}, &m_external));
}
kr::JsExternal::~JsExternal() noexcept
{
}
void kr::JsExternal::_release() noexcept
{
	JsRelease(m_external, nullptr);
}

// class info
kr::_pri_::JsClassInfo::JsClassInfo(Text16 name, size_t parentIdx, CTOR ctor, void (*initMethods)(), bool global) noexcept
{
	static size_t s_indexCounter = 0;
	
	m_index = s_indexCounter++;
	s_classList.add(this);
	m_name = name;
	m_parentIndex = parentIdx;
	m_ctor = ctor;
	m_initMethods = initMethods;
	m_next = nullptr;
	m_isGlobal = global;
}

// class
kr::JsClass kr::JsClass::_createChild(Text16 _name, CTOR ctor) const noexcept
{
	JsScope scope;
	JsRawData name(_name);
	JsRawData cls = InternalTools::createClass(name,  ctor, s_prototypeId);
	scope.add(cls);
	InternalTools::extends(cls, *this, s_prototypeId, s_constructorId);
	return scope.returnValue(cls);
}
void kr::JsClass::setField(Text16 _name, const JsValue &_v) throws(JsException)
{
	JsScope scope;
	JsValue prototype = get((JsPropertyId)s_prototypeId);
	prototype.set(_name, _v);
}
void kr::JsClass::makeField(Text16 _name, JsFilter* _filter) noexcept
{
	JsScope scope;
	debug(); // TODO: implement
	// JsDefineProperty(object, propertyid, propertydes, );
	//prototype->SetAccessor(v8str(_name),
	//	[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
	//		JsFilter* d = getExternal<JsFilter>(_info.Data());
	//		_info.GetReturnValue().Set(_info.This()->GetInternalField(d->m_index));
	//	},
	//	[](Local<String> prop, Local<Value> _value, const PropertyCallbackInfo<void>& _info) {
	//		JsFilter* d = getExternal<JsFilter>(_info.Data());
	//		_info.This()->SetInternalField(d->m_index, Tools::toV8Value(d->filter(Tools::toJSValue(_value))));
	//	}, v8::Handle<External>::New(g_isolate, _filter->m_external), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
}
void kr::JsClass::makeReadOnlyField(Text16 name, int index) noexcept
{
	debug(); // TODO: implement
	class IndexData :public JsExternal
	{
	public:
		int index;
		IndexData(int index) noexcept:index(index) {}
		virtual void remove() noexcept override
		{
			delete this;
		}
	};
	IndexData* data = new IndexData(index);

	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	//prototype->SetAccessor(v8str(name),
	//	[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
	//		IndexData* d = getExternal<IndexData>(_info.Data());
	//		_info.GetReturnValue().Set(_info.This()->GetInternalField(d->index));
	//	}, nullptr, v8::Handle<External>::New(g_isolate, data->m_external), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
}
void kr::JsClass::setAccessor(Text16 _name, JsAccessor* _accessor) noexcept
{
	debug(); // TODO: implement
	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	//prototype->SetAccessor(v8str(_name),
	//	[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
	//		JsAccessor* d = getExternal<JsAccessor>(_info.Data());
	//		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	//	},
	//	[](Local<String> prop, Local<Value> _value, const PropertyCallbackInfo<void>& _info) {
	//		JsAccessor* d = getExternal<JsAccessor>(_info.Data());
	//		d->set(Tools::toJS(_info.This()), Tools::toJSValue(_value));
	//	}, v8::Handle<External>::New(g_isolate, _accessor->m_external), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
	_accessor->Release();
}
void kr::JsClass::setReadOnlyAccessor(Text16 _name, JsAccessor* _accessor) noexcept
{
	debug(); // TODO: implement
	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();

	//prototype->SetAccessor(v8str(_name),
	//	[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
	//		JsAccessor* d = getExternal<JsAccessor>(_info.Data());
	//		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	//	}, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external), (AccessControl)(ALL_CAN_READ));
	_accessor->Release();
}
void kr::JsClass::setIndexAccessor(JsIndexAccessor* _accessor) noexcept
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
void kr::JsClass::setReadOnlyIndexAccessor(JsIndexAccessor* _accessor) noexcept
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
void kr::JsClass::setStaticAccessor(Text16 _name, JsAccessor* _accessor) noexcept
{
	debug(); // TODO: implement
	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//func->SetNativeDataProperty(v8str(_name),
	//	[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
	//		JsAccessor* d = getExternal<JsAccessor>(_info.Data());
	//		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	//	},
	//	[](Local<String> prop, Local<Value> _value, const PropertyCallbackInfo<void>& _info) {
	//		JsAccessor* d = getExternal<JsAccessor>(_info.Data());
	//		d->set(Tools::toJS(_info.This()), Tools::toJSValue(_value));
	//	}, v8::Handle<External>::New(g_isolate, _accessor->m_external), None, Local<AccessorSignature>(), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
}
void kr::JsClass::setStaticReadOnlyAccessor(Text16 _name, JsAccessor* _accessor) noexcept
{
	debug(); // TODO: implement
	JsScope scope;
	//v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	//func->SetNativeDataProperty(v8str(_name),
	//	[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
	//		JsAccessor* d = getExternal<JsAccessor>(_info.Data());
	//		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	//	}, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external), ReadOnly, Local<AccessorSignature>(), (AccessControl)(ALL_CAN_READ));
}
kr::JsValue kr::JsClass::newInstanceRaw(JsArgumentsIn args) const throws(JsException)
{
	JsRawData instance;
	{
		JsScope scope;
		TmpArray<JsValueRef> nargs;
		nargs.push(s_undefinedValue);
		for (const JsValue& arg : args)
		{
			nargs.push(arg.m_data);
		}
		jsthrow(JsConstructObject(m_data, nargs.data(), intact<unsigned short>(nargs.size()), &instance.m_data));
	}
	s_currentScope->add(instance);
	return (JsRawData)instance;
}
kr::JsObject* kr::JsClass::newInstanceRawPtr(JsArgumentsIn args) const throws(JsException)
{
	JsValue obj = newInstanceRaw(args);
	return obj.as<JsObject*>();
}

// runtime
kr::JsRuntime::JsRuntime() noexcept
{
	_assert(!s_runtime);
	JsRuntimeRef runtime;
	jsassert(JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime));
	setRuntime(runtime);
}
kr::JsRuntime::~JsRuntime() noexcept
{
	jsassert(JsDisposeRuntime(s_runtime));
	s_runtime = JS_INVALID_REFERENCE;
	s_sourceContextCounter = 0;
}

kr::JsValue kr::JsRuntime::run(Text16 fileName, Text16 source) throws(JsException)
{
	JsRawData result;
	TText16 buf;
	jsthrow(JsRunScript(szlize(source, &buf), s_sourceContextCounter++, wide(source.data()), &result.m_data));
	s_currentScope->add(result);
	return result;
}
kr::JsValue kr::JsRuntime::global() noexcept
{
	return (JsRawData)s_globalObject;
}

void kr::JsRuntime::setRuntime(const JsRawRuntime &runtime) noexcept
{
	if (s_runtime)
	{
		((JsRuntime*)nullptr)->~JsRuntime();
	}
	s_runtime = runtime;
	_pri_::JsClassInfo& info = *s_classList.begin();
	s_nativeClass = info.get<JsObject>();
	_assert(info.m_name == u"NativeObject");
}
void kr::JsRuntime::gc() noexcept
{
	jsassert(JsCollectGarbage(s_runtime));
}

// context
kr::JsContext::JsContext() noexcept
	:JsContext(InternalTools::createContext())
{
}
kr::JsContext::JsContext(const JsContext& _ctx) noexcept
{
	JsAddRef(m_context = _ctx.m_context, nullptr);
	JsAddRef(m_global = _ctx.m_global, nullptr);
	JsAddRef(m_undefinedValue = _ctx.m_undefinedValue, nullptr);
	JsAddRef(m_trueValue = _ctx.m_trueValue, nullptr);
	JsAddRef(m_falseValue = _ctx.m_falseValue, nullptr);
	JsAddRef(m_nullValue = _ctx.m_nullValue, nullptr);
	JsAddRef(m_prototypeId = _ctx.m_prototypeId, nullptr);
	JsAddRef(m_constructorId = _ctx.m_constructorId, nullptr);

	m_classes = _ctx.m_classes;
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
	jsassert(JsGetGlobalObject(&m_global));
	s_globalObject = m_global;
	jsassert(JsGetUndefinedValue(&m_undefinedValue));
	s_undefinedValue = m_undefinedValue;
	jsassert(JsGetTrueValue(&m_trueValue));
	s_trueValue = m_trueValue;
	jsassert(JsGetFalseValue(&m_falseValue));
	s_falseValue = m_falseValue;
	jsassert(JsGetNullValue(&m_nullValue));
	s_nullValue = m_nullValue;
	jsassert(JsGetPropertyIdFromName(L"prototype", &m_prototypeId));
	s_prototypeId = m_prototypeId;
	jsassert(JsGetPropertyIdFromName(L"constructor", &m_constructorId));
	s_constructorId = m_constructorId;

	{
		JsScope scope;

		for (auto & info : s_classList)
		{
			JsRawData* cls = m_classes.prepare(1);
			JsRawData name(info.m_name);
			*cls = InternalTools::createClass(name, info.m_ctor, m_prototypeId);
			*info.get() = *cls;
			if (info.m_isGlobal)
			{
				jsassert(JsSetIndexedProperty(m_global, name.m_data, cls->m_data));
			}

			if (info.m_parentIndex != -1) InternalTools::extends(*cls, m_classes[info.m_parentIndex], m_prototypeId, m_constructorId);
			info.m_initMethods();
		}
	}
	JsSetCurrentContext(oldctx);
	JsRelease(oldctx, nullptr);
}
kr::JsContext::~JsContext() noexcept
{
	JsContextRef oldctx = InternalTools::getCurrentContext();
	JsSetCurrentContext(m_context);
	for (JsRawData value : m_classes.reverse())
	{
		JsRelease(value.m_data, nullptr);
	}
	JsRelease(m_undefinedValue, nullptr);
	JsRelease(m_trueValue, nullptr);
	JsRelease(m_falseValue, nullptr);
	JsRelease(m_nullValue, nullptr);
	JsRelease(m_prototypeId, nullptr);
	JsRelease(m_constructorId, nullptr);
	JsRelease(m_global, nullptr);
	JsSetCurrentContext(oldctx);
	JsRelease(m_context, nullptr);
	JsRelease(oldctx, nullptr);
}

void kr::JsContext::enter() noexcept
{
	_assert(InternalTools::checkCurrentContext(nullptr));
	JsSetCurrentContext(m_context);
	s_manualScope.create();
	s_globalObject = m_global;
	s_undefinedValue = m_undefinedValue;
	s_trueValue = m_trueValue;
	s_falseValue = m_falseValue;
	s_nullValue = m_nullValue;
	s_prototypeId = m_prototypeId;
	s_constructorId = m_constructorId;
	JsRawData* clsptr = m_classes.data();

	for (_pri_::JsClassInfo& info : s_classList)
	{
		*info.get() = *clsptr++;
	}
}
void kr::JsContext::exit() noexcept
{
	_assert(InternalTools::checkCurrentContext(m_context));
	s_manualScope.remove();
	JsSetCurrentContext(nullptr);
}

#else

EMPTY_SOURCE

#endif