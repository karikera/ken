#include "stdafx.h"

#ifdef WIN32

#include "com.h"


using namespace kr;

ComData::ComData() noexcept
{
	m_ptr = nullptr;
}
void ComData::remove() noexcept
{
	if (m_ptr)
	{
		m_ptr->Release();
		m_ptr = nullptr;
	}
}


#endif
