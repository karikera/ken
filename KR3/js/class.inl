#pragma once

#include "class.h"
#include "arguments.h"
#include "exception.h"

#include <KR3/meta/array.h>

template <typename T>
kr::JsClass kr::JsClassT<T>::createChild() noexcept
{
	return JsClass::createChild<T>();
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setAccessor(const JsPropertyId& name, P(T::*v)) noexcept
{
	JsClass::setAccessorL(name, [v](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr)
			return undefined;
		return (const P&)(_nthis->*v);
		},
		[v](JsValue _this, JsValue _nv) {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr)
			return;
		_nthis->*v = _nv.cast<P>();
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setGetter(const JsPropertyId& name, P(T::*v)) noexcept
{
	JsClass::setAccessorL(name, [v](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (const P&)(_nthis->*v);
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept
{
	JsClass::setAccessorL(name, [get](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setMethodAccessor(const JsPropertyId& name, P(T::* get)(), void(T::* set)(P)) noexcept
{
	JsClass::setAccessorL(name, [get](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
	}, [set](JsValue _this, JsValue nv) {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return;
		return (_nthis->*set)(nv.cast<P>());
	});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setNullableMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept
{
	JsClass::setAccessorL(name, [get](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		return (_nthis->*get)();
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setNullableMethodAccessor(const JsPropertyId& name, P(T::*get)(), void(T::*set)(P)) noexcept
{
	JsClass::setAccessorL(name, [get](JsValue _this)->JsValue {
		T * _nthis = _this.template getNativeObject<T>();
		return (_nthis->*get)();
	}, [set](JsValue _this, JsValue nv) {
		T * _nthis = _this.template getNativeObject<T>();
		return (_nthis->*set)(nv.cast<P>());
	});
}

template <typename T>
void kr::JsClassT<T>::setMethodRaw(const JsPropertyId& name, JsValue(T::* func)(const JsArguments &)) noexcept
{
	setField(name, JsFunction::make([func](const JsArguments & args)->JsValue {
		T * _nthis = args.getThis().getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*func)(args);
	}));
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setIndexAccessor(P(T::* get)(uint32_t)) noexcept
{
	JsClass::setIndexAccessorRaw(JsIndexAccessor::wrap([get](JsValue _this, uint32_t idx)->JsValue {
		T * _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)(idx);
	}));
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setIndexAccessor(P(T::* get)(uint32_t), void(T::* set)(uint32_t, P)) noexcept
{
	JsClass::setIndexAccessorRaw(JsIndexAccessor::wrap([get](JsValue _this, uint32_t idx)->JsValue {
		T * _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)(idx);
	}, [set](JsValue _this, uint32_t idx, JsValue value) {
		T * _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return;
		return (_nthis->*set)(idx, value);
	}));
}

template <typename T>
template <typename RET, typename ... ARGS>
void kr::JsClassT<T>::setMethod(const JsPropertyId& name, RET(T::* func)(ARGS ...)) noexcept
{
	setField(move(name), JsFunction::make([func](const JsArguments & args)->JsValue {
		return JsMeta<decltype(func)>::Call::call(func, args);
	}));
}


template <typename P>
void kr::JsClass::setStaticAccessor(const JsPropertyId& name, P* value) noexcept
{
	setMethodStaticAccessor(name,
		[value](JsValue)->JsValue { return *value; },
		[value](JsValue, JsValue nv) { *value = nv.get<P>(); }
	);
}

template <typename P>
void kr::JsClass::setStaticGetter(const JsPropertyId& name, P* value) noexcept
{
	setStaticGetterL(name,
		[value](JsValue)->JsValue { return *value; }
	);
}

template <typename GET, typename SET>
void kr::JsClass::setAccessorL(const JsPropertyId& name, GET&& get) noexcept
{
	setAccessorRaw(name, JsAccessor::wrap(forward<GET>(get), [](JsValue _this, JsValue _nv) {
			throw JsException(u"Cannot assign to read only property");
		}));
}

template <typename GET, typename SET>
void kr::JsClass::setAccessorL(const JsPropertyId& name, GET&& get, SET&& set) noexcept
{
	setAccessorRaw(name, JsAccessor::wrap(forward<GET>(get), forward<SET>(set)));
}

template <typename GET>
void kr::JsClass::setGetterL(const JsPropertyId& name, GET&& get) noexcept
{
	setGetterRaw(name, JsGetter::wrap(forward<GET>(get)));
}

template <typename GET, typename SET>
void kr::JsClass::setStaticAccessorL(const JsPropertyId& name, GET&& get, SET&& set) noexcept
{
	setStaticAccessorRaw(name, JsAccessor::wrap(forward<GET>(get), forward<SET>(set)));
}

template <typename GET>
void kr::JsClass::setStaticGetterL(const JsPropertyId& name, GET&& get) noexcept
{
	setStaticGetterRaw(name, JsGetter::wrap(forward<GET>(get)));
}

template <typename LAMBDA>
void kr::JsClass::setMethodRaw(const JsPropertyId& name, LAMBDA&& _lambda) noexcept
{
	setField(name, JsFunction::make(forward<LAMBDA>(_lambda)));
}

template <typename T>
kr::JsClass kr::JsClass::createChild(Text16 _name) noexcept
{
	return _createChild(_name,
		[](const JsArguments & arguments)->JsObject * { return _new T(arguments); }
	);
}
template <typename T, typename P>
void kr::JsClass::setMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept
{
	setGetter(name, [get](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		});
}
template <typename T, typename P>
void kr::JsClass::setMethodAccessor(const JsPropertyId& name, P(T:: * get)(), void(T:: * set)(P)) noexcept
{
	setAccessor(name, [get](JsValue _this)->JsValue {
		T* _nthis = _this.template getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		}, [set](JsValue _this, JsValue nv) {
			T* _nthis = _this.template getNativeObject<T>();
			if (_nthis == nullptr) return;
			return (_nthis->*set)(nv.cast<P>());
		});
}

template <typename T, typename RET, typename ... ARGS>
void kr::JsClass::setMethod(const JsPropertyId& name, RET(T::* func)(ARGS ...)) noexcept
{
	set(name, JsFunction([func](const JsArguments& args)->JsValue {
		return JsMeta<decltype(func)>::Call::call(func, args);
		}));
}
template <typename LAMBDA>
void kr::JsClass::setStaticMethod(const JsPropertyId& name, LAMBDA&& lambda) noexcept
{
	set(name, JsFunction::makeT(forward<LAMBDA>(lambda)));
}
template <typename LAMBDA>
void kr::JsClass::setStaticMethodRaw(const JsPropertyId& name, LAMBDA&& lambda) noexcept
{
	set(name, JsFunction::make(forward<LAMBDA>(lambda)));
}

// 객체를 생성합니다
// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
template <typename T>
template <typename ... ARGS>
T* kr::JsClassT<T>::newInstance(const ARGS & ... args) const throws(JsException)
{
	meta::array<JsValue, sizeof...(args)> list = { args ... };
	JsValue value = newInstanceRaw(View<JsValue>(list, endof(list)));
	return value.getNativeObject<T>();
}
