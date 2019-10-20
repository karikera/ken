#pragma once
#include "v8data.h"

#ifdef __KR3_INCLUDED
inline kr::JsAny kr::V8Object::callRaw(Text16 name, JsArgumentsIn args) const
{
	return get(name).cast<V8Function>().callRaw(*this, args);
}
template <typename Class> inline void kr::V8Object::setClass()
{
	setClass(Class::getClassName(), &Class::classObject);
}
template <typename LAMBDA> inline void kr::V8Object::setFunctionRaw(Text16 name, LAMBDA lambda)
{
	set(name, JsFunction(lambda));
}

template <typename RET, typename ... ARGS> inline RET kr::V8Object::call(Text16 name, const ARGS& ... args) const
{
	JsAny nargs[] = { JsAny(args) ... };
	return callRaw(name, JsArgumentsIn(nargs)).cast<RET>();
}
template <typename LAMBDA> inline void kr::V8Object::setFunction(Text16 name, LAMBDA lambda)
{
	set(name, JsFunction::make(lambda));
}

#endif

inline kr::JsObject<kr::NativeObject>::JsObject(const JsArguments& args)
	:V8Object(args.getThis())
{
}
inline kr::JsObject<kr::NativeObject>::~JsObject() noexcept
{
}
#ifdef __KR3_INCLUDED
inline kr::V8Object kr::JsObject<kr::NativeObject>::newInstanceRaw(JsArgumentsIn args)
{
	return getClassObject()->newInstanceRaw(args);
}
#endif
inline kr::V8Class* kr::JsObject<kr::NativeObject>::getClass()
{
	return &classObject;
}
inline void kr::JsObject<kr::NativeObject>::finallize() noexcept
{
#ifdef __KR3_INCLUDED
	alloc<alignof(NativeObject)>::free(this);
#else
#endif
}


template <typename LAMBDA>
void kr::V8Data::setWeak(LAMBDA lambda)
{
	_setWeak([this, lambda] {
		lambda();
		setEmpty();
	});
}
inline kr::V8Data & kr::V8Data::operator =(const V8Data & value)
{
	this->~V8Data();
	new(this) V8Data(value);
	return *this;
}
inline bool kr::V8Data::operator ==(const V8Data & o) const
{
	return equals(o);
}
inline bool kr::V8Data::operator !=(const V8Data & o) const
{
	return !equals(o);
}

template <typename T>
inline void kr::V8Value::_set(const V8Value & any)
{
	if (!any.isEmpty() && any.is<T>())
	{
		*static_cast<V8Value*>(this) = any;
	}
	else
	{
		setEmpty();
	}
}

inline kr::V8Handle<v8::Object>::V8Handle()
{
}
inline kr::V8Handle<v8::Object>::V8Handle(nullptr_t)
{
}
inline kr::V8Handle<v8::Object>::V8Handle(const V8Handle &obj)
	:V8Value(obj)
{
}
inline kr::V8Handle<v8::Object>::V8Handle(const V8Value & any)
{
	_set<v8::Object>(any);
}

inline kr::V8Handle<v8::Array>::V8Handle()
{
}
inline kr::V8Handle<v8::Array>::V8Handle(std::nullptr_t)
{
}
inline kr::V8Handle<v8::Array>::V8Handle(const V8Handle &obj)
	:V8Object(obj)
{
}
inline kr::V8Handle<v8::Array>::V8Handle(const V8Value & any)
{
	_set<v8::Array>(any);
}

inline kr::V8Handle<v8::Function>::V8Handle()
{
}
inline kr::V8Handle<v8::Function>::V8Handle(nullptr_t)
{
}
inline kr::V8Handle<v8::Function>::V8Handle(const V8Handle &obj)
	:V8Object(obj)
{
}
inline kr::V8Handle<v8::Function>::V8Handle(const V8Value & any)
{
	_set<v8::Function>(any);
}

template <typename RET, typename T, typename ... ARGS> inline RET kr::V8Handle<v8::Function>::call(T* _this, const ARGS & ... args) const
{
	meta::array<JsAny, sizeof...(args)> list = { args ... };
	return callRaw(*_this, View<JsAny>(list, endof(list))).cast<RET>();
}
template <typename RET, typename T, typename ... ARGS> inline RET kr::V8Handle<v8::Function>::safeCall(T* _this, const ARGS & ... args) const
{
	if (isEmpty())
		return _pri_::JsCast::defaultValue<RET>();
	meta::array<JsAny, sizeof...(args)> list = { args ... };
	return callRaw(*_this, View<JsAny>(list, endof(list))).cast<RET>();
}
// 객체의 함수를 호출합니다.
template <typename ... ARGS> inline kr::V8Object kr::V8Handle<v8::Function>::callNew(const ARGS & ... args) const
{
	return callNewRaw({ args ... });
}