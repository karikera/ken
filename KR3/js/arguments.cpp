#include "stdafx.h"
#include "arguments.h"
#include "object.h"

using namespace kr;


JsArguments::JsArguments(const JsValue& _callee, const JsValue& _this, JsArgumentsIn args) noexcept
	:m_callee(_callee), m_this(_this), m_arguments(args)
{
}
JsArguments::JsArguments(const JsValue& _callee, const JsValue& _this, size_t sz) noexcept
	: m_callee(_callee), m_this(_this)
{
	m_arguments.resize(sz);
}
JsArguments::JsArguments(JsArguments&& _move) noexcept
{
	m_arguments = move(_move.m_arguments);
	m_callee = move(_move.m_callee);
	m_this = move(_move.m_this);
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
JsValue& JsArguments::operator [](size_t i) noexcept
{
	_assert(i <= m_arguments.size());
	return m_arguments[i];
}
const JsValue& JsArguments::operator [](size_t i) const noexcept
{
	static const JsValue undefined;
	if (i >= m_arguments.size()) return undefined;
	return m_arguments[i];
}
size_t JsArguments::size() const noexcept
{
	return m_arguments.size();
}

const JsValue& JsArguments::getCallee() const noexcept
{
	return m_callee;
}
const JsValue& JsArguments::getThis() const noexcept
{
	return m_this;
}
JsValue* JsArguments::begin() noexcept
{
	return m_arguments.data();
}
JsValue* JsArguments::end() noexcept
{
	return m_arguments.data() + m_arguments.size();
}
const JsValue* JsArguments::begin() const noexcept
{
	return m_arguments.data();
}
const JsValue* JsArguments::end() const noexcept
{
	return m_arguments.data() + m_arguments.size();
}
