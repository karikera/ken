#pragma once

#include "class.h"
#include <KR3/meta/array.h>

#ifdef __KR3_INCLUDED

template <typename T>
kr::JsClass kr::JsClassT<T>::createChild() noexcept
{
	return JsClass::createChild<T>();
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setAccessor(Text16 _name, P(T::*v)) noexcept
{
	auto get = [v](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr)
			return undefined;
		return (const P&)(_nthis->*v);
	};
	auto set = [v](JsObject _this, JsValue _nv) {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr)
			return;
		_nthis->*v = _nv.cast<P>();
	};
	JsClass::setAccessor(_name, get, set);
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setReadOnlyAccessor(Text16 _name, P(T::*v)) noexcept
{
	auto get = [v](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (const P&)(_nthis->*v);
	};
	JsClass::setAccessor(_name, get);
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setMethodAccessor(Text16 _name, P(T::*get)()) noexcept
{
	JsClass::setReadOnlyAccessor(_name, [get](JsObject _this)->JsValue{
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
	});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setMethodAccessor(Text16 _name, P(T::*get)(), void(T::*set)(P)) noexcept
{
	JsClass::setAccessor(_name, [get](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
	}, [set](JsObject _this, JsValue nv) {
		T * _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return;
		return (_nthis->*set)(nv.cast<P>());
	});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setNullableMethodAccessor(Text16 _name, P(T::*get)()) noexcept
{
	JsClass::setReadOnlyAccessor(_name, [get](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		return (_nthis->*get)();
	});
}

template <typename T>
template <typename P>
void kr::JsClassT<T>::setNullableMethodAccessor(Text16 _name, P(T::*get)(), void(T::*set)(P)) noexcept
{
	JsClass::setAccessor(_name, [get](JsObject _this)->JsValue {
		T * _nthis = _this.getNativeObject<T>();
		return (_nthis->*get)();
	}, [set](JsObject _this, JsValue nv) {
		T * _nthis = _this.getNativeObject<T>();
		return (_nthis->*set)(nv.cast<P>());
	});
}

template <typename T>
void kr::JsClassT<T>::setMethodRaw(Text16 _name, JsValue(T::* func)(const JsArguments &)) noexcept
{
	set(_name, JsFunction([func](const JsArguments & args)->JsValue {
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
void kr::JsClassT<T>::setMethod(Text16 _name, RET(T::* func)(ARGS ...)) noexcept
{
	setField(_name, JsFunction::make([func](const JsArguments & args)->JsValue {
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
template <typename LAMBDA>
void kr::JsClass::setStaticMethodRaw(Text16 _name, LAMBDA lambda) noexcept
{
	setStatic(_name, JsFunction(lambda));
}
template <typename T, typename P>
void kr::JsClass::setMethodAccessor(Text16 _name, P(T::* get)()) noexcept
{
	setReadOnlyAccessor(_name, [get](JsObject _this)->JsValue {
		T* _nthis = _this.getNativeObject<T>();
		if (_nthis == nullptr) return undefined;
		return (_nthis->*get)();
		});
}
template <typename T, typename P>
void kr::JsClass::setMethodAccessor(Text16 _name, P(T:: * get)(), void(T:: * set)(P)) noexcept
{
	setAccessor(_name, [get](JsObject _this)->JsValue {
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
void kr::JsClass::setMethod(Text16 _name, RET(T:: * func)(ARGS ...)) noexcept
{
	set(_name, JsFunction([func](const JsArguments & args)->JsValue {
		return JsMeta<decltype(func)>::Call::call(func, args);
		}));
}

template <typename LAMBDA>
void kr::JsClass::setStaticMethod(Text16 _name, LAMBDA lambda) noexcept
{
	setStatic(_name, JsFunction::make(lambda));
}

#endif

// 객체를 생성합니다
// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
template <typename T>
template <typename ... ARGS>
T* kr::JsClassT<T>::newInstance(const ARGS & ... args) const noexcept
{
	meta::array<JsValue, sizeof...(args)> list = { args ... };
	return static_cast<T*>(newInstanceRawPtr(View<JsValue>(list, endof(list))));
}
