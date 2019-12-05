#include "stdafx.h"
#include "arguments.h"
#include "object.h"

using namespace kr;


JsArguments::JsArguments(const JsValue& _callee, const JsValue& _this, JsRawDataValue* args, size_t argn) noexcept
	: m_callee(_callee), m_this(_this), m_args(args), m_argn(argn)
{
}
JsArguments::JsArguments(JsArguments&& _move) noexcept
	:m_callee(move(_move.m_callee)), 
	m_this(move(_move.m_this)),
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
size_t JsArguments::size() const noexcept
{
	return m_argn;
}

const JsValue& JsArguments::getCallee() const noexcept
{
	return m_callee;
}
const JsValue& JsArguments::getThis() const noexcept
{
	return m_this;
}
