#include "stdafx.h"
#include "autoptr.h"

kr::autoptr::autoptr(nullptr_t) noexcept
	:m_ptr(nullptr)
{
}
kr::autoptr::autoptr(intptr_t dat) noexcept
	:m_ptr((void*)dat)
{
}
kr::autoptr::autoptr(uintptr_t dat) noexcept
	:m_ptr((void*)dat)
{
}
kr::autoptr & kr::autoptr::operator=(nullptr_t) noexcept
{
	m_ptr = nullptr;
	return *this;
}
bool kr::autoptr::operator ==(nullptr_t) const noexcept
{
	return m_ptr == nullptr;
}
bool kr::autoptr::operator !=(nullptr_t) const noexcept
{
	return m_ptr != nullptr;
}
bool kr::operator ==(nullptr_t, const autoptr& _this) noexcept
{
	return _this == nullptr;
}
bool kr::operator !=(nullptr_t, const autoptr& _this) noexcept
{
	return _this != nullptr;
}
