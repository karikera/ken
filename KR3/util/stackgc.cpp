#include "stdafx.h"
#include "stackgc.h"

kr::StackGC::StackGC() noexcept
{
	m_last = nullptr;
}
kr::StackGC::~StackGC() noexcept
{
	StackAllocator * alloc = StackAllocator::getInstance();
	Head * node = m_last;
	while (node != nullptr)
	{
		node->destruct(node+1);
		Head * prev = node->prev;
		alloc->free(node);
		node = prev;
	}
}
