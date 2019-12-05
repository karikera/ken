#pragma once

#include "class.h"
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
	auto get = [v](JsValue _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr)
			return undefined;
		return (const P&)(_nthis->*v);
	};
	auto set = [v](JsValue _this, JsValue _nv) {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr)
			return;
		_nthis->*v = _nv.cast<P>();
	};
	JsClass::setAccessorL(name, get, set);
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setGetter(const JsPropertyId& name, P(T::*v)) noexcept
{
	auto get = [v](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (const P&)(_nthis->*v);
	};
	JsClass::setAccessor(name, get);
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept
{
	JsClass::setReadOnlyAccessor(name, [get](JsObject _this)->JsValue {
		T* _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setMethodAccessor(const JsPropertyId& name, P(T::* get)(), void(T::* set)(P)) noexcept
{
	JsClass::setAccessor(name, [get](JsObject _this)->JsValue {
		T* _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		}, [set](JsObject _this, JsValue nv) {
			T* _nthis = _this.getNativeObject<T>();
			if (_nthis == nullptr) return;
			return (_nthis->*set)(nv.cast<P>());
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setNullableMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept
{
	JsClass::setReadOnlyAccessor(name, [get](JsObject _this)->JsValue {
		T* _nthis = _this.getNativeObject<T>();
		return (_nthis->*get)();
		});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setNullableMethodAccessor(const JsPropertyId& name, P(T::*get)(), void(T::*set)(P)) noexcept
{
	JsClass::setAccessor(name, [get](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		return (_nthis->*get)();
	}, [set](JsObject _this, JsValue nv) {
		T * _nthis = _this.getNativeObject<T>();
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
	JsClass::setReadOnlyIndexAccessor(JsIndexAccessor([get](JsObject _this, uint32_t idx)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)(idx);
	}));
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setIndexAccessor(P(T::* get)(uint32_t), void(T::* set)(uint32_t, P)) noexcept
{
	JsClass::setIndexAccessor(JsIndexAccessor([get](JsObject _this, uint32_t idx)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)(idx);
	}, [set](JsObject _this, uint32_t idx, JsValue value) {
		T * _nthis = _this.getNativeObject<T>();
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
	setGetter(name, [get](JsObject _this)->JsValue {
		T* _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		});
}
template <typename T, typename P>
void kr::JsClass::setMethodAccessor(const JsPropertyId& name, P(T:: * get)(), void(T:: * set)(P)) noexcept
{
	setAccessor(name, [get](JsObject _this)->JsValue {
		T* _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		}, [set](JsObject _this, JsValue nv) {
			T* _nthis = _this.getNativeObject<T>();
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
