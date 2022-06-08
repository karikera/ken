#pragma once

#include <KR3/main.h>

namespace kr {
	template <typename T, uint32_t size>
	class AtomicStaticQueue {
	public:
		bool enqueue(T value) noexcept;
		T dequeue(T onFail) noexcept;

	private:
		atomic<T> m_queue[size];
		atomic<uint32_t> m_head;
		atomic<uint32_t> m_rear;
		atomic<uint32_t> m_writing;
	};
}

template <typename T, uint32_t size>
bool kr::AtomicStaticQueue<T, size>::enqueue(T value) noexcept {
	uint32_t old, next;
	do {
		old = m_writing;
		next = (old + 1) % size;
		if (m_head == next) return false;
	} while (!m_writing.compare_exchange_weak(old, next));

	m_queue[old] = value;
	
	uint32_t old2;
	do {
		old2 = old;
	} while (!m_rear.compare_exchange_weak(old2, old));
	return true;
}
template <typename T, uint32_t size>
T kr::AtomicStaticQueue<T, size>::dequeue(T onFail) noexcept {
	T value;
	uint32_t old;
	do {
		old = m_head;
		if (m_rear == old) return onFail;
		value = m_queue[old];
	} while (!m_head.compare_exchange_weak(old, (old + 1) % size));
	return value;
}
