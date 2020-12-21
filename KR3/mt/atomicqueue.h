#pragma once

#include <KR3/mt/criticalsection.h>

namespace kr
{
	class AtomicQueue;

	class AtomicQueueNode
	{
		friend AtomicQueue;
	private:
		atomic<AtomicQueueNode*> m_next;
	};

	class AtomicQueue
	{
	public:
		void enqueue(AtomicQueueNode* param) noexcept;
		AtomicQueueNode* dequeue() noexcept;

	private:
		atomic<AtomicQueueNode*> m_first;
		atomic<AtomicQueueNode*> m_last;
	};
}