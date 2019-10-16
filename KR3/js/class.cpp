#include "stdafx.h"
#include "class.h"

using namespace kr;

JsClass::JsClass() noexcept
{
}
JsClass::JsClass(const JsRawData& _copy) noexcept
	:JsValue(_copy)
{
}
JsClass::~JsClass() noexcept
{
}
