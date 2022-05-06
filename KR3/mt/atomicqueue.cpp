#include "stdafx.h"
#include "atomicqueue.h"

using namespace kr;

AtomicQueueNode::AtomicQueueNode() noexcept
	:m_ref(2), m_done(false), m_next(nullptr)
{
}
AtomicQueueNode::~AtomicQueueNode() noexcept
{
}
intptr_t AtomicQueueNode::release() noexcept
{
	intptr_t ref = --m_ref;
	if (ref != 0) return ref;
	delete this;
	return 0;
}
AtomicQueueNode::AtomicQueueNode(int) noexcept
	:m_ref(2), m_done(true), m_next(nullptr)
{
}

AtomicQueue::AtomicQueue() noexcept
	:AtomicQueue(_new AtomicQueueNode(0))
{
}
AtomicQueue::~AtomicQueue() noexcept
{
	for (;;)
	{
		auto pair = dequeue();
		if (pair.first == nullptr) break;
		pair.first->release();
	}
	AtomicQueueNode* node = m_rear.load();
	delete node;
}
AtomicQueue::AtomicQueue(AtomicQueueNode* endnode) noexcept
	:m_front(endnode), m_rear(endnode), m_size(0)
{
}
intptr_t AtomicQueue::enqueue(AtomicQueueNode* param) noexcept
{
	AtomicQueueNode* axis;
	AtomicQueueNode* expected;

	do
	{
		axis = m_rear;
		expected = nullptr;
	} while (!axis->m_next.compare_exchange_weak(expected, param));
	do
	{
		expected = axis;
	} while (!m_rear.compare_exchange_weak(expected, param));
	expected->release();

	return ++m_size;
}
pair<AtomicQueueNode*, intptr_t> AtomicQueue::peek() noexcept {
	AtomicQueueNode* front;
	AtomicQueueNode* next;

	for (;;)
	{
		do
		{
			front = m_front;
			next = front->m_next;
			if (next == nullptr)
			{
				if (front->m_done == false)
				{
					++front->m_ref;
					intptr_t size = --m_size;
					return { front, size };
				}
				else
				{
					return { nullptr, m_size };
				}
			}
		} while (!m_front.compare_exchange_weak(front, next));

		if (front->m_done)
		{
			front->release();
			continue;
		}
		intptr_t size = --m_size;
		return { front, size };
	}
	return { m_front, m_size };
}
pair<AtomicQueueNode*, intptr_t> AtomicQueue::dequeue() noexcept
{
	AtomicQueueNode* front;
	AtomicQueueNode* next;

	for (;;)
	{
		do
		{
			front = m_front;
			next = front->m_next;
			if (next == nullptr)
			{
				bool old = false;
				if (front->m_done.compare_exchange_strong(old, true))
				{
					++front->m_ref;
					intptr_t size = --m_size;
					return { front, size };
				}
				else
				{
					return { nullptr, m_size };
				}
			}
		} while (!m_front.compare_exchange_weak(front, next));

		if (front->m_done)
		{
			front->release();
			continue;
		}
		intptr_t size = --m_size;
		return { front, size };
	}
}