#include "stdafx.h"
#include "atomicqueue.h"

using namespace kr;

void AtomicQueue::enqueue(AtomicQueueNode* param) noexcept
{
	AtomicQueueNode* last;
	do
	{
		last = m_last;
		param->m_next = last;
	} while (!m_last.compare_exchange_weak(last, param));
	if (last == nullptr) m_first = param;
}
AtomicQueueNode* AtomicQueue::dequeue() noexcept
{
	AtomicQueueNode* first;
	do
	{
		first = m_first;
		if (first == nullptr) return nullptr;
	} while (!m_first.compare_exchange_weak(first, first->m_next));
	m_last.compare_exchange_strong(first, nullptr);
	return first;
}