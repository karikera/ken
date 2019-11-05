#pragma once

#include "meta.h"
#include "undefined.h"

template <typename CLASS, typename RET, typename ... ARGS>
template <size_t ... idx>
template <typename FUNC>
kr::JsValue kr::JsMetaLambda<RET(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(const FUNC& fn, const JsArguments & args) throws(JsException)
{
	size_t n = args.size();
	return fn((idx < n ? args[idx].cast<ARGS>() : _pri_::JsCast::defaultValue<ARGS>())...);
}

template <typename CLASS, typename ... ARGS>
template <size_t ... idx>
template <typename FUNC>
kr::JsValue kr::JsMetaLambda<void(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(FUNC fn, const JsArguments & args) throws(JsException)
{
	size_t n = args.size();
	fn((idx < n ? args[idx].cast<ARGS>() : _pri_::JsCast::defaultValue<ARGS>())...);
	return undefined;
}

template <typename RET, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<RET(*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(RET(*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	size_t n = args.size();
	return fn((idx < n ? args[idx].cast<ARGS>() : _pri_::JsCast::defaultValue<ARGS>())...);
}

template <typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<void(*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(void(*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	size_t n = args.size();
	fn((idx < n ? args[idx].cast<ARGS>() : _pri_::JsCast::defaultValue<ARGS>())...);
	return undefined;
}

template <typename CLASS, typename RET, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<RET(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(RET(CLASS::*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	CLASS * _this = args.getThis().getNativeObject<CLASS>();
	if (_this == nullptr) throw JsException(u"Invalid this object");
	size_t n = args.size();
	return (_this->*fn)((idx < n ? args[idx].cast<ARGS>() : _pri_::JsCast::defaultValue<ARGS>())...);
}

template <typename CLASS, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<void(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(void(CLASS::*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	CLASS * _this = args.getThis().getNativeObject<CLASS>();
	if (_this == nullptr) throw JsException(u"Invalid this object");
	size_t n = args.size();
	(_this->*fn)((idx < n ? args[idx].cast<ARGS>() : _pri_::JsCast::defaultValue<ARGS>())...);
	return undefined;
}