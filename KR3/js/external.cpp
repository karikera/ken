#include "stdafx.h"
#include "external.h"

using namespace kr;

void JsExternal::AddRef() noexcept
{
	m_reference++;
}
void JsExternal::Release() noexcept
{
	m_reference--;
	if (m_reference == 0) _release();
}
