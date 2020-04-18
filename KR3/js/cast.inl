#pragma once

#include "cast.h"

template <typename To>
To* kr::_pri_::JsCast::ToOuter<To*, kr::JsObjectRawData>::toOuter(const kr::JsObjectRawData& _value) noexcept
{
	if (!_value.instanceOf(JsObject::classObject)) return nullptr;
	return dynamic_cast<To*>(_value.template as<JsObject*>());
}