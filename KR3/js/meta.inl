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
	return fn(args.at<ARGS>(idx)...);
}

template <typename CLASS, typename ... ARGS>
template <size_t ... idx>
template <typename FUNC>
kr::JsValue kr::JsMetaLambda<void(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(FUNC fn, const JsArguments & args) throws(JsException)
{
	fn(args.at<ARGS>(idx)...);
	return undefined;
}

template <typename RET, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<RET(*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(RET(*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	return fn(args.at<ARGS>(idx)...);
}

template <typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<void(*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(void(*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	fn(args.at<ARGS>(idx)...);
	return undefined;
}

template <typename CLASS, typename RET, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<RET(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(RET(CLASS::*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	CLASS * _this = args.getThis().getNativeObject<CLASS>();
	if (_this == nullptr) throw JsException(TSZ16() << u"this is not " << CLASS::className);
	return (_this->*fn)(args.at<ARGS>(idx)...);
}

template <typename CLASS, typename ... ARGS>
template <size_t ... idx>
kr::JsValue kr::JsMeta<void(CLASS::*)(ARGS ...)>
	::numunwrap<kr::meta::numlist<idx ...>>
	::call(void(CLASS::*fn)(ARGS ...), const JsArguments & args) throws(JsException)
{
	CLASS * _this = args.getThis().getNativeObject<CLASS>();
	if (_this == nullptr) throw JsException(TSZ16() << u"this is not " << CLASS::className);
	(_this->*fn)(args.at<ARGS>(idx)...);
	return undefined;
}