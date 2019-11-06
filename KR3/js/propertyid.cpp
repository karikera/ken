#include "stdafx.h"
#include "propertyid.h"

using namespace kr;

JsPropertyId& JsPropertyId::operator =(const JsPropertyId& name) noexcept
{
	this->~JsPropertyId();
	new(this) JsPropertyId(name);
	return *this;
}

JsPropertyId& JsPropertyId::operator =(JsPropertyId&& name) noexcept
{
	this->~JsPropertyId();
	new(this) JsPropertyId(move(name));
	return *this;
}
