#include "stdafx.h"
#include "function.h"

using namespace kr;


JsFunction::JsFunction() noexcept
{
	m_data = nullptr;
}
bool JsFunction::isEmpty() const noexcept
{
	return m_data == nullptr;
}
JsFunction::~JsFunction() noexcept
{
	if (m_data == nullptr) return;
	m_data->Release();
}
JsFunction::JsFunction(const JsFunction& func) noexcept
{
	m_data = func.m_data;
	m_data->AddRef();
}
JsFunction::JsFunction(JsFunction&& func) noexcept
{
	m_data = func.m_data;
	func.m_data = nullptr;
}

JsValue JsFunction::create() const noexcept
{
	_assert(m_data != nullptr);
	return m_data->create();
}

JsValue JsFunction::call(const JsArguments& args) const noexcept
{
	return m_data->call(args);
}
JsFunction& JsFunction::operator =(const JsFunction& _copy) noexcept
{
	this->~JsFunction();
	new(this) JsFunction(_copy);
	return *this;
}
JsFunction& JsFunction::operator =(JsFunction&& _copy) noexcept
{
	this->~JsFunction();
	new(this) JsFunction(std::move(_copy));
	return *this;
}
bool JsFunction::operator ==(const JsFunction& o) const noexcept
{
	return m_data == o.m_data;
}
bool JsFunction::operator !=(const JsFunction& o) const noexcept
{
	return m_data != o.m_data;
}
