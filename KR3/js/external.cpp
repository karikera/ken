#include "stdafx.h"
#include "external.h"

using namespace kr;

void JsExternalData::AddRef() noexcept
{
	m_reference++;
}
void JsExternalData::Release() noexcept
{
	m_reference--;
	if (m_reference == 0) _release();
}
