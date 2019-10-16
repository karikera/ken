#include <include/v8.h>
#include <include/libplatform/libplatform.h>

#ifdef _DEBUG
#pragma comment(lib, "v8d.lib")
#else
#pragma comment(lib, "v8.lib")
#endif


#define CBS_EASYV8_INTERNAL
#include "ken-v8.h"

#include <new>
#include <string>
#include <unordered_map>


#pragma comment(lib, "winmm.lib")

using namespace std;
using namespace kr;
using namespace v8;

static_assert(sizeof(Persistent<Value>) == sizeof(void*), "size unmatch");

namespace
{
	struct
	{
		alignas(JsClass<NativeObject>) char buffer[sizeof(JsClass<NativeObject>)];

		inline JsClass<NativeObject> * operator ->()
		{
			return (JsClass<NativeObject>*)buffer;
		}
		inline operator JsClass<NativeObject> *()
		{
			return (JsClass<NativeObject>*)buffer;
		}
	} g_ezclsBuffer;
	
}
JsClass<NativeObject> & JsObject<NativeObject>::classObject = *g_ezclsBuffer;


namespace
{
	Platform* g_platform = nullptr;
	Isolate * g_isolate = nullptr;
	Persistent<String> g_sharing;
	v8::Persistent<ObjectTemplate> s_tmplGlobal;
	std::vector<V8Class*> s_classes;

#ifndef NDEBUG
	bool s_destroyed;
	int s_entercontext;
#endif

	JsString jsstr(v8::Handle<Value> _str) noexcept;
	v8::Handle<String> v8str(const char16 * _str, size_t _length) noexcept;
	template <size_t sz>
	v8::Handle<String> v8str(const char16(&_str)[sz]) noexcept;
	v8::Handle<Value> v8str(const JsString& _str) noexcept;

	JsString jsstr(v8::Handle<Value> _str) noexcept
	{
		v8::String::Value strvalue(_str->ToString());
		const char16 * strraw = (char16*)(uint16_t*)*strvalue;

		if (strraw != nullptr)
			return JsText(strraw, strvalue.length());
		else
			return (JsText)u"[[Invalid String]]";
	}
	v8::Handle<String> v8str(const char16 * _str, size_t _length) noexcept
	{
		assert(_length <= 0x7fffffff);
		return String::NewFromTwoByte(g_isolate, (uint16_t*)_str, String::kNormalString, (int)_length);
	}
	v8::Handle<String> v8str(JsText text) noexcept
	{
		return v8str(text.data, text.length);
	}
	template <size_t sz>
	v8::Handle<String> v8str(const char16(&_str)[sz]) noexcept
	{
		return v8str(_str, sz - 1);
	}
	v8::Handle<Value> v8str(const JsString& _str) noexcept
	{
		if (_str.c_str() == nullptr) return Null(g_isolate);
		return v8str(_str.c_str(), _str.length());
	}

	template <typename T> 
	T* getExternal(v8::Handle<Value> _v) noexcept
	{
		return (T*)_v.As<External>()->Value();
	}
	struct Init : public v8::ArrayBuffer::Allocator
	{
		Init() noexcept;

		virtual void* Allocate(size_t _length) noexcept override {
			void* data = AllocateUninitialized(_length);
			return data == NULL ? data : memset(data, 0, _length);
		}
		virtual void* AllocateUninitialized(size_t _length) noexcept override
		{
			return malloc(_length);
		}
		virtual void Free(void* data, size_t) noexcept override
		{
			free(data);
		}
	};
	Init __init;
	
	class RethrowV8Exception
	{
	private:
		TryCatch m_try_catch;
	public:

		void test() // JsException
		{
			if(m_try_catch.HasCaught())
				throw JsException(&m_try_catch);
		}

		template <typename T>
		v8::Handle<T> test(v8::Handle<T> _v) // JsException
		{
			if (m_try_catch.HasCaught())
				throw JsException(&m_try_catch);
			assert(!_v.IsEmpty());
			return _v;
		}
	};
}

struct kr::_pri_::InternalTools
{
	static v8::Handle<FunctionTemplate> getEZObj()
	{
		return v8::Handle<FunctionTemplate>::New(g_isolate, JsObject<NativeObject>::classObject._handle());
	}
	static JsAny toJSValue(v8::Handle<Value> _v);
	static v8::Handle<Data> toV8Tmpl(const JsAny & _value);
	static v8::Handle<Value> toV8Value(const JsAny & _value);
	static JsArguments toJSArgs(const FunctionCallbackInfo<Value> & _fci);
	static std::vector<Local<Value>> toV8Args(JsArgumentsIn _fci);
	static void nativeConstructorCallback(const FunctionCallbackInfo<Value>& _info);
	static void nativeFunctionCallback(const FunctionCallbackInfo<Value>& _info);

	template <typename T>
	static void reset(V8Handle<T> * dest) noexcept
	{
		cls->_handle().Reset();
	}
	template <typename T>
	static V8Handle<T> toJS(Local<T> _v)
	{
		V8Handle<T> obj;
		obj.m_value.Reset(g_isolate, _v);
		return obj;
	}
	template <typename T>
	static v8::Handle<T> toV8(const V8Handle<T> & _v) noexcept
	{
		return v8::Handle<T>::New(g_isolate, _v._handle());
	}
	template <typename T>
	static void fromV8(V8Handle<T> * dest, const v8::Handle<T> & v) noexcept
	{
		cls->_handle().Reset(g_isolate, v);
	}
};

typedef _pri_::InternalTools Tools;

void * kr::ezv8_malloc(size_t sz)
{
	return __init.Allocate(sz);
}
void kr::ezv8_free(void * ptr)
{
	__init.Free(ptr, 0);
}

Init::Init() noexcept
{
	V8::InitializeICU();
	g_platform = platform::CreateDefaultPlatform();
	V8::InitializePlatform(g_platform);
	V8::Initialize();

	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = this;

	g_isolate = Isolate::New(create_params);
	g_isolate->Enter();
	
	HandleScope scope(g_isolate);
	
	v8::Handle<String> v8windowstr = v8str(u"Window");
	v8::Handle<FunctionTemplate> globalcls = FunctionTemplate::New(g_isolate);
	globalcls->SetClassName(v8windowstr);

	v8::Handle<ObjectTemplate> tmplGlobal = globalcls->InstanceTemplate();
	tmplGlobal->Set(v8windowstr, globalcls);
	s_tmplGlobal.Reset(g_isolate, tmplGlobal);

	g_sharing.Reset(g_isolate, v8str(u"sharing"));
}

V8Data::V8Handle()
{
}
V8Data::V8Handle(const V8Handle & _value)
{
	m_value.Reset(g_isolate, _value.m_value);
}
V8Data::V8Handle(V8Handle && _value)
{
	m_value.Reset(g_isolate, _value.m_value);
	_value.m_value.Reset();
}
V8Data::~V8Data()
{
#ifndef NDEBUG
	assert(!s_destroyed || m_value.IsEmpty()); // 데이터의 메모리가 누수되었다.
#endif
	m_value.Reset();
}
void V8Data::setEmpty()
{
	m_value.Reset();
}
bool V8Data::isEmpty() const
{
	return m_value.IsEmpty();
}
void V8Data::setWeak()
{
	m_value.SetWeak(this, [](const WeakCallbackInfo<V8Data> &data) {
		V8Data* _this = data.GetParameter();
		_this->m_value.Reset();
	}, WeakCallbackType::kParameter);
}
void V8Data::_setWeak(JsLambda callback)
{
	m_value.SetWeak(callback.detach(), [](const WeakCallbackInfo<JsLambda::Data> &data) {
		JsLambda::Data* _lambda = data.GetParameter();
		_lambda->call();
		_lambda->remove();
	}, WeakCallbackType::kParameter);
}
void V8Data::clearWeak()
{
	V8Value * _this = m_value.ClearWeak<V8Value>();
	if (_this == this) return;
	delete (JsLambda::Data*)_this;
}
bool V8Data::equals(const V8Data& o) const
{
	return m_value == o.m_value;
}

kr::JsString::JsString(const V8Handle<v8::Value> & obj)
	:JsString(obj.toString())
{
}

V8Handle<Private>::V8Handle()
{
	HandleScope scope(g_isolate);
	m_value.Reset(g_isolate, Private::New(g_isolate));
}
V8Handle<Private>::~V8Handle()
{
#ifndef NDEBUG
	assert(!s_destroyed || m_value.IsEmpty()); // private 데이터의 메모리가 누수되었다.
#endif
	m_value.Reset();
}
inline v8::Persistent<v8::Private>& V8Handle<Private>::_handle()
{
	return (v8::Persistent<v8::Private>&)m_value;
}
inline const v8::Persistent<v8::Private>& V8Handle<Private>::_handle() const
{
	return (v8::Persistent<v8::Private>&)m_value;
}

//V8Value::V8Handle(const NativeObject * no)
//{
//	if (no != nullptr)
//	{
//		m_value.Reset(g_isolate, no->m_value);
//	}
//}
template <> bool V8Value::is<Function>() const
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Data> obj = v8::Handle<Data>::New(g_isolate, m_value);
	return obj.As<Value>()->IsFunction();
}
template <> bool V8Value::is<v8::Array>() const
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Data> obj = v8::Handle<Data>::New(g_isolate, m_value);
	return obj.As<Value>()->IsArray();
}
template <> bool V8Value::is<Object>() const
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Data> obj = v8::Handle<Data>::New(g_isolate, m_value);
	return obj.As<Value>()->IsObject();
}
JsString V8Value::toString() const
{
	HandleScope scope(g_isolate);
	return jsstr(v8::Handle<Data>::New(g_isolate, m_value).As<Value>());
}

void JsExternalData::_toWeak() noexcept
{
	m_external.SetWeak(this, [](const WeakCallbackInfo<JsExternalData> &data) {
		data.GetParameter()->remove();
	}, WeakCallbackType::kParameter);
}
JsExternalData::JsExternalData() noexcept
{
	m_reference = 0;
	HandleScope scope(g_isolate);
	v8::Handle<External> edata = External::New(g_isolate, this);
	m_external.Reset(g_isolate, edata);
}
JsExternalData::~JsExternalData() noexcept
{
#ifndef NDEBUG
	assert(!s_destroyed || m_external.IsEmpty()); // 네이티브 함수의 메모리가 누수되었다.
#endif
	m_external.Reset();
}
V8Function JsFunction::Data::getV8Value() noexcept
{
	return Tools::toJS(Function::New(g_isolate, &Tools::nativeFunctionCallback, v8::Handle<External>::New(g_isolate, m_external)));
}

JsAny Tools::toJSValue(v8::Handle<Value> _v)
{
	if (_v->IsNull()) return nullptr;
	if(_v->IsUndefined()) return undefined;
	else if (_v->IsInt32()) return _v.As<Int32>()->Value();
	else if (_v->IsNumber()) return _v.As<v8::Number>()->Value();
	else if (_v->IsBoolean()) return _v.As<Boolean>()->Value();
	else if (_v->IsExternal()) return _v.As<External>()->Value();
	else if (_v->IsFunction()) return Tools::toJS(_v.As<Function>());
	else if (_v->IsArray()) return Tools::toJS(_v.As<v8::Array>());
	else if (_v->IsObject() && !_v->IsStringObject()) return Tools::toJS(_v.As<Object>());
	else return jsstr(_v);
}
v8::Handle<Data> Tools::toV8Tmpl(const JsAny & _value)
{
	switch (_value.getType())
	{
	case JsTypeNull: return Null(g_isolate);
	case JsTypeBoolean: return Boolean::New(g_isolate, _value.as<bool>());
	case JsTypeInteger: return Int32::New(g_isolate, _value.as<int>());
	case JsTypeFloat: return v8::Number::New(g_isolate, _value.as<double>());
	case JsTypeString:
	{
		JsString str = _value.as<JsString>();
		if (str == nullptr) 
			return Null(g_isolate);
		return v8str(str);
	}
	case JsTypeExternal: return External::New(g_isolate, _value.as<void*>());
	case JsTypeV8Value: return v8::Handle<Data>::New(g_isolate, _value.as<V8Value>().m_value);
	case JsTypeNativeFunction:
		{
			const JsFunction & nativefunc = _value.as<JsFunction>();
			v8::Handle<FunctionTemplate> func = FunctionTemplate::New(g_isolate, &Tools::nativeFunctionCallback, v8::Handle<External>::New(g_isolate, nativefunc.m_func->m_external));
			return func;
		}
	case JsTypeUndefined:
		return v8::Undefined(g_isolate);
	default:
		assert(!"Invalid type"); // 올 수 없는 값
		return v8::Undefined(g_isolate);
	}
}
v8::Handle<Value> Tools::toV8Value(const JsAny & _value)
{
	switch (_value.getType())
	{
	case JsTypeNull: return Null(g_isolate);
	case JsTypeBoolean: return Boolean::New(g_isolate, _value.as<bool>());
	case JsTypeInteger: return Int32::New(g_isolate, _value.as<int>());
	case JsTypeFloat: return v8::Number::New(g_isolate, _value.as<double>());
	case JsTypeString: return v8str(_value.as<JsString>());
	case JsTypeExternal: return External::New(g_isolate, _value.as<void*>());
	case JsTypeV8Value:
	{
		v8::Handle<Value> data = v8::Handle<Data>::New(g_isolate, _value.as<V8Value>().m_value).As<Value>();
		if(data.IsEmpty()) return Null(g_isolate);
		return data;
	}
	case JsTypeNativeFunction:
	{
		const JsFunction & nativefunc = _value.as<JsFunction>();
		if(nativefunc.m_func == nullptr)
			return Null(g_isolate);
		v8::Handle<Function> func = Function::New(g_isolate, &Tools::nativeFunctionCallback, v8::Handle<External>::New(g_isolate, nativefunc.m_func->m_external));
		return func;
	}
	case JsTypeUndefined:
		return v8::Undefined(g_isolate);
	default:
		assert(!"Invalid Type"); // 올 수 없는 값
		return v8::Undefined(g_isolate);
	}
}
JsArguments Tools::toJSArgs(const FunctionCallbackInfo<Value> & _fci)
{
	int argn = _fci.Length();
	JsArguments args(Tools::toJS(_fci.Callee()), Tools::toJS(_fci.This()), argn);
	for (int i = 0; i < (int)args.size(); i++)
	{
		args[i] =  toJSValue(_fci[i]);
	}
	return args;
}
vector<Local<Value>> Tools::toV8Args(JsArgumentsIn args)
{
	std::vector<Local<Value>> v8args(args.size());

	const JsAny* src = args.begin();
	const JsAny* src_end = args.end();
	
	Local<Value>* dst = v8args.data();


	while (src != src_end)
	{
		*dst++ = toV8Value(*src++);
	}
	assert(args.size() <= 0x7fffffff);
	return v8args;
}
void Tools::nativeConstructorCallback(const FunctionCallbackInfo<Value>& _info)
{
	if (!_info.IsConstructCall())
	{
		_info.GetReturnValue().Set(g_isolate->ThrowException(v8::Exception::TypeError(v8str(u"Need new operator"))));
		return;
	}

	using CTOR = V8Handle<FunctionTemplate>::CTOR;
	CTOR ctor = (CTOR)_info.Data().As<External>()->Value();
	try
	{
		NativeObject * jsobj = ctor(Tools::toJSArgs(_info));
		jsobj->setWeak();
	}
	catch (JsException & e)
	{
		_info.GetReturnValue().Set(g_isolate->ThrowException(v8::Exception::Error(v8str((JsText)e.message))));
	}
}
void Tools::nativeFunctionCallback(const FunctionCallbackInfo<Value>& _info)
{
	JsFunction::Data * data = getExternal<JsFunction::Data>(_info.Data());
	try
	{
		JsAny retValue = data->call(toJSArgs(_info));
		_info.GetReturnValue().Set(toV8Value(retValue));
	}
	catch (JsException & e)
	{
		_info.GetReturnValue().Set(g_isolate->ThrowException(v8::Exception::Error(v8str((JsText)e.message))));
	}
}

V8Object V8Handle<Function>::callNewRaw(JsArgumentsIn args) const // JsException
{
	HandleScope scope(g_isolate);
	v8::Handle<Function> func = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Function>();
	std::vector<Local<Value>> v8args = Tools::toV8Args(args);

	RethrowV8Exception _try_catch;
	Local<Object> ret = func.As<Function>()->NewInstance((int)args.size(), v8args.data());
	return Tools::toJS(_try_catch.test(ret));
}
JsAny V8Handle<Function>::callRaw(const V8Object &_this, JsArgumentsIn args) const // JsException
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, _this.m_value).As<Value>().As<Object>();
	v8::Handle<Function> func = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Function>();
	std::vector<Local<Value>> v8args = Tools::toV8Args(args);

	RethrowV8Exception _try_catch;
	Local<Value> ret = func->Call(obj, (int)args.size(), v8args.data());
	return Tools::toJSValue(_try_catch.test(ret));
}

V8Handle<v8::Object> kr::V8Handle<v8::Object>::newInstance()
{
	HandleScope scope(g_isolate);
	return Tools::toJS(v8::Object::New(g_isolate));
}

void V8Handle<Object>::setClass(JsText _name, V8Handle<v8::FunctionTemplate> * _value)
{
	assert(!m_value.IsEmpty());
	HandleScope scope(g_isolate);

	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	v8::Handle<FunctionTemplate> fntmpl = v8::Handle<FunctionTemplate>::New(g_isolate, _value->_handle());
	v8::Handle<Function> func = fntmpl->GetFunction();
	assert(!func.IsEmpty());
	obj->Set(v8str(_name), func);
}
void V8Handle<Object>::set(JsText _name, JsAny _value)
{
	assert(!m_value.IsEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	obj->Set(v8str(_name), Tools::toV8Value(_value));
}
JsAny V8Handle<Object>::get(JsText _name) const
{
	assert(!m_value.IsEmpty());
	
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	return Tools::toJSValue(obj->Get(v8str(_name)));
}
bool kr::V8Handle<v8::Object>::remove(JsText _name) const
{
	assert(!m_value.IsEmpty());

	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	return obj->Delete(v8str(_name));
}
void V8Handle<Object>::set(const V8Private & p, JsAny _value)
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	obj->SetPrivate(v8::Handle<Private>::New(g_isolate,p._handle()), Tools::toV8Value(_value));
}
JsAny V8Handle<Object>::get(const V8Private & p) const
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	return Tools::toJSValue(obj->GetPrivate(v8::Handle<Private>::New(g_isolate, p._handle())));
}
bool kr::V8Handle<v8::Object>::remove(const V8Private & p) const
{
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	return obj->DeletePrivate(v8::Handle<Private>::New(g_isolate, p._handle()));
}
void V8Handle<Object>::set(uint32_t idx, JsAny _value)
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	obj->Set(idx, Tools::toV8Value(_value));
}
JsAny V8Handle<Object>::get(uint32_t idx) const
{
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	return Tools::toJSValue(obj->Get(idx));
}
void V8Handle<Object>::setInternal(int idx, JsAny _value) const
{
	assert(idx >= 0 );
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	assert(idx < obj->InternalFieldCount());
	obj->SetInternalField(idx, Tools::toV8Value(_value));
}
JsAny V8Handle<Object>::getInternal(int idx) const
{
	assert(idx >= 0);
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	assert(idx < obj->InternalFieldCount());
	return Tools::toJSValue(obj->GetInternalField(idx));
}
int V8Handle<Object>::internalFieldCount() const
{
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	return obj->InternalFieldCount()-1;
}
NativeObject * V8Handle<Object>::getNativeObject() const
{
	assert (!m_value.IsEmpty());
	HandleScope scope(g_isolate);

	v8::Handle<Value> val = v8::Handle<Data>::New(g_isolate, m_value).As<Value>();
	if (!val->IsObject()) return nullptr;

	if (Tools::getEZObj()->HasInstance(val))
	{
		v8::Handle<Object> obj = val.As<Object>();
		return (NativeObject*)obj->GetAlignedPointerFromInternalField(0);
	}
	return nullptr;
}

V8Handle<v8::Array> V8Handle<v8::Array>::newInstance(uint32_t _length)
{
	HandleScope scope(g_isolate);
	return Tools::toJS(v8::Array::New(g_isolate, _length));
}
uint32_t V8Handle<v8::Array>::getLength() const
{
	HandleScope scope(g_isolate);
	assert(!isEmpty());
	return v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<v8::Array>()->Length();
}

NativeObject::NativeObject(const JsArguments & args)
	:JsObject(args)
{
	HandleScope scope(g_isolate);
	v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
	obj->SetAlignedPointerInInternalField(0, this);
}
NativeObject::~NativeObject()
{
	if (!m_value.IsEmpty())
	{
		HandleScope scope(g_isolate);
		clearWeak();
		v8::Handle<Object> obj = v8::Handle<Data>::New(g_isolate, m_value).As<Value>().As<Object>();
		obj->SetAlignedPointerInInternalField(0, nullptr);
	}
}
void NativeObject::setWeak()
{
	m_value.SetWeak(static_cast<V8Value*>(this), [](const WeakCallbackInfo<V8Value> &data) {
		NativeObject * _this = static_cast<NativeObject*>(data.GetParameter());
		_this->m_value.Reset();
		_this->finallize();
	}, WeakCallbackType::kParameter);
}

bool NativeObject::deleted() noexcept
{
	return m_value.IsEmpty();
}

JsException::JsException(v8::TryCatch * _try_catch)
{
	message = jsstr(_try_catch->Exception());

	v8::Handle<v8::Message> message = _try_catch->Message();
	if (!message.IsEmpty())
	{
		fileName = jsstr(message->GetScriptOrigin().ResourceName());
		lineNumber = message->GetLineNumber();
		sourceLine = jsstr(message->GetSourceLine());
		startColumn = message->GetStartColumn();
		endColumn = message->GetEndColumn();
		stackTrace = jsstr(_try_catch->StackTrace());
	}
	else
	{
		lineNumber = 0;
		startColumn = 0;
		endColumn = 0;
	}
}

V8Handle<FunctionTemplate>::V8Handle()
{
}
V8Handle<FunctionTemplate>::V8Handle(const V8Class & _copy)
{
	_handle().Reset(g_isolate, _copy._handle());
}
V8Handle<FunctionTemplate>::~V8Handle()
{
}
V8Function V8Handle<FunctionTemplate>::getFunction()
{
	assert(!isEmpty());
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> functmpl = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<Function> func = functmpl->GetFunction();
	return Tools::toJS(func);
}

void V8Handle<FunctionTemplate>::setInternalFieldCount(int _count)
{
	assert(_count >= 0);
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> instance = func->InstanceTemplate();
	instance->SetInternalFieldCount(_count);
}
int V8Handle<FunctionTemplate>::getInternalFieldCount()
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> instance = func->InstanceTemplate();
	return instance->InternalFieldCount();
}
void V8Handle<FunctionTemplate>::set(JsText _name, JsAny _v) // JsException
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	RethrowV8Exception try_catch;
	prototype->Set(v8str(_name), Tools::toV8Tmpl(_v));
	try_catch.test();
}
void V8Handle<FunctionTemplate>::setStatic(JsText _name, JsAny _v) // JsException
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	RethrowV8Exception _try_catch;
	func->Set(v8str(_name), Tools::toV8Tmpl(_v));
	_try_catch.test();
}
void V8Handle<FunctionTemplate>::makeField(JsText _name, JsFilter* _filter)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	prototype->SetAccessor(v8str(_name),
		[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
		JsFilter * d = getExternal<JsFilter>(_info.Data());
		_info.GetReturnValue().Set(_info.This()->GetInternalField(d->m_index));
	},
		[](Local<String> prop, Local<Value> _value, const PropertyCallbackInfo<void>& _info) {
		JsFilter * d = getExternal<JsFilter>(_info.Data());
		_info.This()->SetInternalField(d->m_index, Tools::toV8Value(d->filter(Tools::toJSValue(_value))));
	}, v8::Handle<External>::New(g_isolate, _filter->m_external), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
}
void V8Handle<FunctionTemplate>::makeReadOnlyField(JsText name, int index)
{
	class IndexData:public JsExternalData
	{
	public:
		virtual void remove() noexcept override
		{
			delete this;
		}
		int index;
	};
	IndexData * data = new IndexData;
	data->index = index;

	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	prototype->SetAccessor(v8str(name),
		[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
		IndexData * d = getExternal<IndexData>(_info.Data());
		_info.GetReturnValue().Set(_info.This()->GetInternalField(d->index));
	}, nullptr, v8::Handle<External>::New(g_isolate, data->m_external), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
}
void V8Handle<FunctionTemplate>::setAccessor(JsText _name, JsAccessor *_accessor)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	prototype->SetAccessor(v8str(_name),
		[](Local<String> prop, const PropertyCallbackInfo<Value>& _info){
			JsAccessor * d = getExternal<JsAccessor>(_info.Data());
			_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
		}, 
		[](Local<String> prop, Local<Value> _value, const PropertyCallbackInfo<void>& _info) {
			JsAccessor * d = getExternal<JsAccessor>(_info.Data());
			d->set(Tools::toJS(_info.This()), Tools::toJSValue(_value));
		}, v8::Handle<External>::New(g_isolate, _accessor->m_external), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
	_accessor->Release();
}
void V8Handle<FunctionTemplate>::setReadOnlyAccessor(JsText _name, JsAccessor *_accessor)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();

	prototype->SetAccessor(v8str(_name),
		[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
		JsAccessor * d = getExternal<JsAccessor>(_info.Data());
		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	}, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external), (AccessControl)(ALL_CAN_READ));
	_accessor->Release();
}
void V8Handle<FunctionTemplate>::setIndexAccessor(JsIndexAccessor *_accessor)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();

	prototype->SetIndexedPropertyHandler(
		[](uint32_t idx, const PropertyCallbackInfo<Value>& _info) {
		JsIndexAccessor * d = getExternal<JsIndexAccessor>(_info.Data());
		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()), idx)));
	},
		[](uint32_t idx, Local<Value> _value, const PropertyCallbackInfo<Value>& _info) {
		JsIndexAccessor * d = getExternal<JsIndexAccessor>(_info.Data());
		d->set(Tools::toJS(_info.This()), idx, Tools::toJSValue(_value));
	}, nullptr, nullptr, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external));
	_accessor->Release();
}
void V8Handle<FunctionTemplate>::setReadOnlyIndexAccessor(JsIndexAccessor *_accessor)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	v8::Handle<ObjectTemplate> prototype = func->PrototypeTemplate();
	
	prototype->SetIndexedPropertyHandler(
		[](uint32_t idx, const PropertyCallbackInfo<Value>& _info) {
		JsIndexAccessor * d = getExternal<JsIndexAccessor>(_info.Data());
		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()), idx)));
	}, nullptr, nullptr, nullptr, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external));
	_accessor->Release();
}
void V8Handle<FunctionTemplate>::setStaticAccessor(JsText _name, JsAccessor * _accessor)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	func->SetNativeDataProperty(v8str(_name),
		[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
		JsAccessor * d = getExternal<JsAccessor>(_info.Data());
		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	},
		[](Local<String> prop, Local<Value> _value, const PropertyCallbackInfo<void>& _info) {
		JsAccessor * d = getExternal<JsAccessor>(_info.Data());
		d->set(Tools::toJS(_info.This()), Tools::toJSValue(_value));
	}, v8::Handle<External>::New(g_isolate, _accessor->m_external), None, Local<AccessorSignature>(), (AccessControl)(ALL_CAN_READ | ALL_CAN_WRITE));
}
void V8Handle<FunctionTemplate>::setStaticReadOnlyAccessor(JsText _name, JsAccessor * _accessor)
{
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> func = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	func->SetNativeDataProperty(v8str(_name),
		[](Local<String> prop, const PropertyCallbackInfo<Value>& _info) {
		JsAccessor * d = getExternal<JsAccessor>(_info.Data());
		_info.GetReturnValue().Set(Tools::toV8Value(d->get(Tools::toJS(_info.This()))));
	}, nullptr, v8::Handle<External>::New(g_isolate, _accessor->m_external), ReadOnly, Local<AccessorSignature>(), (AccessControl)(ALL_CAN_READ));
}
V8Object V8Handle<FunctionTemplate>::newInstanceRaw(JsArgumentsIn args) const // JsException
{
#ifndef NDEBUG
	assert(s_entercontext != 0);
#endif
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> fn = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	vector<Local<Value>> v8args = Tools::toV8Args(args);

	RethrowV8Exception _try_catch;
	v8::Handle<Object> ret = fn->GetFunction()->NewInstance((int)v8args.size(), v8args.data());
	return Tools::toJS(_try_catch.test(ret));
}
NativeObject* V8Handle<FunctionTemplate>::newInstanceRawPtr(JsArgumentsIn args) const // JsException
{
#ifndef NDEBUG
	assert(s_entercontext != 0);
#endif
	HandleScope scope(g_isolate);
	v8::Handle<FunctionTemplate> fn = v8::Handle<FunctionTemplate>::New(g_isolate, _handle());
	vector<Local<Value>> v8args = Tools::toV8Args(args);

	RethrowV8Exception _try_catch;
	Local<Object> ret = fn->GetFunction()->NewInstance((int)v8args.size(), v8args.data());
	return (NativeObject*)(_try_catch.test(ret))->GetAlignedPointerFromInternalField(0);
}

JsCode::JsCode() noexcept
{
}
JsCode::JsCode(JsText fileName, JsText source) // JsException
{
	HandleScope scope(g_isolate);

	RethrowV8Exception _try_catch;
	v8::Handle<Script> script = Script::Compile(v8str(source), v8str(fileName));
	m_script.Reset(g_isolate, _try_catch.test(script));
}
JsCode::~JsCode() noexcept
{
	m_script.Reset();
}
JsCode::JsCode(const JsCode &_copy) noexcept
{
	m_script.Reset(g_isolate, _copy.m_script);
}
bool JsCode::empty() const noexcept
{
	return m_script.IsEmpty();
}

JsContext::JsContext() noexcept
{
	HandleScope scope(g_isolate);
	v8::Handle<ObjectTemplate> tmplGlobal = v8::Handle<ObjectTemplate>::New(g_isolate, s_tmplGlobal);

	v8::Handle<Context> context = Context::New(g_isolate, nullptr, tmplGlobal);
	v8::Handle<String> token = v8::Handle<String>::New(g_isolate, g_sharing);
	context->SetSecurityToken(token);

	Context::Scope context_scope(context);
	m_context.Reset(g_isolate, context);
}
JsContext::JsContext(const JsContext & _ctx) noexcept
{
	m_context.Reset(g_isolate, _ctx.m_context);
}
JsContext::~JsContext() noexcept
{
	m_context.Reset();
}
JsAny JsContext::run(const JsCode &code)
{
	assert(!code.empty());

	HandleScope scope(g_isolate);
	TryCatch _try_catch;
	
	v8::Handle<Script> script = v8::Handle<Script>::New(g_isolate, code.m_script);
	v8::Handle<Value> result = script->Run();
	if (result.IsEmpty()) throw JsException(&_try_catch);
	assert(!_try_catch.HasCaught());
	return Tools::toJSValue(result);
}
V8Object JsContext::global()
{
	HandleScope scope(g_isolate);
	return Tools::toJS(v8::Handle<Context>::New(g_isolate, m_context)->Global());
}
void JsContext::enter()
{
#ifndef NDEBUG
	s_entercontext++;
#endif
	HandleScope scope(g_isolate);
	v8::Handle<Context>::New(g_isolate, m_context)->Enter();
}
void JsContext::exit()
{
#ifndef NDEBUG
	assert(s_entercontext != 0);
	s_entercontext--;
#endif
	HandleScope scope(g_isolate);
	v8::Handle<Context>::New(g_isolate, m_context)->Exit();
}

void JsContext::gc() noexcept
{
	g_isolate->LowMemoryNotification();
	while (g_isolate->IdleNotification(10000));
	//g_isolate->IdleNotificationDeadline(10000);
	//g_isolate->AdjustAmountOfExternalAllocatedMemory(-1000000000);
	//g_isolate->RequestGarbageCollectionForTesting(Isolate::kFullGarbageCollection);
}
void JsContext::terminate() noexcept
{
	{
		auto iter = s_classes.rbegin();
		auto end = s_classes.rend();
		while (iter != end)
		{
			(*iter)->~V8Class();
			iter++;
		}
	}
	s_classes = std::vector<V8Class*>();

	g_sharing.Reset();
	s_tmplGlobal.Reset();
	gc();

	g_isolate->Exit();
	g_isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete g_platform;
	
#ifndef NDEBUG
	s_destroyed = true;
#endif
}