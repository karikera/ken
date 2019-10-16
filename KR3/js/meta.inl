#pragma once

#include "meta.h"
#include "undefined.h"

template <typename CLASS, typename RET, typename ... ARGS>
template <size_t ... idx>
template <typename FUNC>
kr::JsValue kr::JsMetaLambda<RET(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(FUNC fn, const JsArguments & args)
{
	return fn((args[idx].cast<ARGS>())...);
}

template <typename CLASS, typename ... ARGS>
template <size_t ... idx>
template <typename FUNC>
kr::JsValue kr::JsMetaLambda<void(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(FUNC fn, const JsArguments & args)
{
	fn((args[idx].cast<ARGS>())...);
	return undefined;
}

template <typename RET, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<RET(*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(RET(*fn)(ARGS ...), const JsArguments & args)
{
	return fn((args[idx].cast<ARGS>())...);
}

template <typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<void(*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(void(*fn)(ARGS ...), const JsArguments & args)
{
	fn((args[idx].cast<ARGS>())...);
	return undefined;
}

template <typename CLASS, typename RET, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<RET(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(RET(CLASS::*fn)(ARGS ...), const JsArguments & args)
{
	CLASS * _this = args.getThis().getNativeObject<CLASS>();
	if (_this == nullptr) throw JsException(u"Invalid this object");
	return (_this->*fn)((args[idx].cast<ARGS>())...);
}

template <typename CLASS, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<void(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(void(CLASS::*fn)(ARGS ...), const JsArguments & args)
{
	CLASS * _this = args.getThis().getNativeObject<CLASS>();
	if (_this == nullptr) throw JsException(u"Invalid this object");
	(_this->*fn)((args[idx].cast<ARGS>())...);
	return undefined;
}