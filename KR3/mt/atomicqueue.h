#pragma once

#include <KR3/mt/criticalsection.h>

namespace kr
{
	class AtomicQueue;

	class AtomicQueueNode
	{
		friend AtomicQueue;
	public:
		AtomicQueueNode() noexcept;
		virtual ~AtomicQueueNode() noexcept;
		intptr_t release() noexcept;

	private:
		AtomicQueueNode(int) noexcept;

		atomic<AtomicQueueNode*> m_next;
		atomic<uint32_t> m_ref;
		atomic<bool> m_done;
	};

	class AtomicQueue
	{
	public:
		AtomicQueue() noexcept;
		~AtomicQueue() noexcept;
		intptr_t enqueue(AtomicQueueNode* param) noexcept;
		pair<AtomicQueueNode*, intptr_t> peek() noexcept;
		pair<AtomicQueueNode*, intptr_t> dequeue() noexcept;

	private:
		AtomicQueue(AtomicQueueNode* endnode) noexcept;

		atomic<AtomicQueueNode*> m_front;
		atomic<AtomicQueueNode*> m_rear;
		atomic<intptr_t> m_size;
	};
}