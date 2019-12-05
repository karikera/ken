#include "stdafx.h"
#include "persistent.h"

using namespace kr;

JsPersistent& JsPersistent::operator =(const JsRawData& value) noexcept
{
	this->~JsPersistent();
	new(this) JsPersistent(value);
	return *this;
}
JsPersistent& JsPersistent::operator =(const JsPersistent& obj) noexcept
{
	this->~JsPersistent();
	new(this) JsPersistent(obj);
	return *this;
}
JsPersistent& JsPersistent::operator =(JsPersistent&& obj) noexcept
{
	this->~JsPersistent();
	new(this) JsPersistent(move(obj));
	return *this;
}

JsPersistent::JsPersistent(nullptr_t) noexcept
	:JsPersistent()
{
}
JsPersistent& JsPersistent::operator =(nullptr_t) noexcept
{
	this->~JsPersistent();
	new(this) JsPersistent();
	return *this;
}

JsWeak& JsWeak::operator =(const JsRawData& value) noexcept
{
	this->~JsWeak();
	new(this) JsWeak(value);
	return *this;
}
JsWeak& JsWeak::operator =(const JsWeak& obj) noexcept
{
	this->~JsWeak();
	new(this) JsWeak(obj);
	return *this;
}
JsWeak& JsWeak::operator =(JsWeak&& obj) noexcept
{
	this->~JsWeak();
	new(this) JsWeak(move(obj));
	return *this;
}
JsWeak::JsWeak(nullptr_t) noexcept
	:JsWeak()
{
}
JsWeak& JsWeak::operator =(nullptr_t) noexcept
{
	this->~JsWeak();
	new(this) JsWeak();
	return *this;
}
