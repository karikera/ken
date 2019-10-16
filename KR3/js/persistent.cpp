#include "stdafx.h"
#include "persistent.h"

using namespace kr;

JsPersistent& JsPersistent::operator =(const JsRawData& value) noexcept
{
	this->~JsPersistent();
	new(this) JsPersistent(value);
	return *this;
}
JsPersistent& JsPersistent::operator =(const JsValue& value) noexcept
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