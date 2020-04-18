#pragma once

#include "value.h"

template <size_t size, typename LAMBDA>
void kr::JsValue::setMethod(const char16(&name)[size], LAMBDA&& func) const noexcept
{
	set(name, JsFunction::makeT(forward<LAMBDA>(func)));
}
template <size_t size, typename LAMBDA>
void kr::JsValue::setMethodRaw(const char16(&name)[size], LAMBDA&& func) const noexcept
{
	set(name, JsFunction::make(forward<LAMBDA>(func)));
}