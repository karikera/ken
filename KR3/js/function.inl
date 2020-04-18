#pragma once

#include "function.h"
#include "arguments.h"

template <typename RET, typename ... ARGS>
RET kr::JsFunctionT<RET(ARGS ...)>::call(const JsValue& _this, const ARGS& ... args) const throws(JsException)
{
	JsArgumentsAllocated jsargs(*this, _this, sizeof ... (args));

	size_t i = sizeof ... (args) - 1;
	unpackR(jsargs.set(i--, args));

	return JsFunction::call(jsargs).template get<RET>();
}
