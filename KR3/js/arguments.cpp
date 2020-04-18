#include "stdafx.h"
#include "arguments.h"
#include "object.h"

using namespace kr;


JsArguments::JsArguments(const JsValue& _this, JsRawDataValue* args, size_t argn) noexcept
	: m_this(_this), m_args(args), m_argn(argn)
{
}
JsArguments::JsArguments(JsArguments&& _move) noexcept
	:m_this(move(_move.m_this)),
	m_args(_move.m_args),
	m_argn(_move.m_argn)
{
}
JsArguments::~JsArguments() noexcept
{
}

JsArguments& JsArguments::operator =(JsArguments&& _move) noexcept
{
	this->~JsArguments();
	new(this) JsArguments(move(_move));
	return *this;
}
JsValue JsArguments::operator [](size_t i) const noexcept
{
	_assert(i < m_argn);
	return (JsValue)(JsRawData)m_args[i];
}
void JsArguments::set(size_t idx, const JsValue& value) noexcept
{
	(JsRawData&)m_args[idx] = value;
}
size_t JsArguments::size() const noexcept
{
	return m_argn;
}

const JsValue& JsArguments::getThis() const noexcept
{
	return m_this;
}
