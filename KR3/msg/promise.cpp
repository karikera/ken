#include "stdafx.h"
#include "promise.h"
#include "poster.h"

#include <stdio.h>

using namespace kr;

PromiseRaw::PromiseRaw() noexcept
{
	m_state = Pending;
	m_ending = false;
	m_next = nullptr;
	m_sibling = nullptr;
	m_psibling = &m_sibling;
}
PromiseRaw::~PromiseRaw() noexcept
{
}
void PromiseRaw::_resolveCommit() noexcept
{
	return _setState(Resolved);
}
void PromiseRaw::_rejectCommit() noexcept
{
	return _setState(Rejected);
}
void PromiseRaw::onThen(PromiseRaw * from) noexcept
{
	_assert(!"not implemented");
}
void PromiseRaw::onKatch(PromiseRaw * from) noexcept
{
	_assert(!"not implemented");
}
void PromiseRaw::_deleteCascade() noexcept
{
	if (m_sibling) m_sibling->_deleteCascade();
	if (m_next) m_next->_deleteCascade();
	delete this;
}
void PromiseRaw::_setState(State state) noexcept
{
	_assert(m_state == State::Pending);
	m_state = state;
	PromisePump * pump = PromisePump::getInstance();
	*pump->m_pprocess = this;
	pump->m_pprocess = &m_sibling;
}
void PromiseRaw::_addNext(PromiseRaw * next) noexcept
{
	m_ending = false;
	if (m_next)
	{
		_assert((uintptr_t)m_next % 4 == 0);
		*m_next->m_psibling = next;
		m_next->m_psibling = &next->m_sibling;
	}
	else
	{
		_assert((uintptr_t)next % 4 == 0);
		m_next = next;
	}
}
void PromiseRaw::_readdNext(PromiseRaw * next) noexcept
{
	_assert(next->m_state == Pending);
	next->m_sibling = nullptr;
	next->m_psibling = &next->m_sibling;
	_addNext(next);
}
std::exception_ptr* PromiseRaw::_rejectValue() noexcept
{
	static_assert(sizeof(PromiseRaw) % alignof(std::exception_ptr) == 0, "alignment match");
	return (std::exception_ptr*)(this + 1);
}
void PromiseRaw::_rejectException(std::exception_ptr data) noexcept
{
	new(_rejectValue()) std::exception_ptr(move(data));
	PromiseRaw::_rejectCommit();
}
void PromiseRaw::_reject() noexcept
{
	_rejectException(std::current_exception());
}

void Promise<void>::connect(DeferredPromise<void>* prom) noexcept
{
	_addNext(prom);
}
PromiseVoid * Promise<void>::resolve() noexcept
{
	PromiseVoid * prom = _new PromiseVoid();
	prom->_resolve();
	return prom;
}
PromiseVoid * Promise<void>::rejectException(std::exception_ptr data) noexcept
{
	PromiseVoid * prom = _new PromiseVoid();
	prom->_rejectException(move(data));
	return prom;
}
PromiseVoid * Promise<void>::reject() noexcept
{
	return reject(std::current_exception());
}
Promise<void>* Promise<void>::all(View<Promise<void>*> proms) noexcept
{
	size_t count = proms.size();
	if (count == 0) return Promise<void>::resolve();
	class Prom :public Promise<void> {
	public:
		using Promise<void>::_resolve;
		size_t count = 0;
	};
	auto * prom = new Prom;
	prom->count = count;
	for (Promise<void> * one_of_proms : proms)
	{
		one_of_proms->then([prom]() {
			prom->count--;
			if (prom->count == 0) prom->_resolve();
		});
	}
	return prom;
}
void Promise<void>::_resolve() noexcept
{
	_resolveCommit();
}
void * Promise<void>::_resolveValue() noexcept
{
	return nullptr;
}

PromisePump::PromisePump() noexcept
{
	m_pprocess = &m_process;
}
PromisePump::~PromisePump() noexcept
{
	_assert(m_process == nullptr); // all tasks must complete
}
PromisePump* PromisePump::getInstance() noexcept
{
#ifdef __EMSCRIPTEN__
	static thread_local PromisePump pump;
	return &pump;
#else
	EventPump* manager = EventPump::getInstance();
	return &manager->m_prompump;
#endif

}
bool PromisePump::empty() noexcept
{
	return m_process == nullptr;
}
void PromisePump::clear() noexcept
{
	PromiseRaw* process = m_process;
	if (process != nullptr)
	{
		m_process = nullptr;
		m_pprocess = &m_process;
		process->_deleteCascade();
	}
}
void PromisePump::process() noexcept
{
	for (;;)
	{
		PromiseRaw* process = m_process;
		if (process == nullptr) return;
		m_process = nullptr;
		m_pprocess = &m_process;

		do
		{
			PromiseRaw* processNext = process->m_sibling;
			PromiseRaw::State state = process->m_state;
			process->m_ending = true;
			PromiseRaw* next = process->m_next;
			process->m_next = nullptr;

			while (next)
			{
				_assert((uintptr_t)next % alignof(PromiseRaw) == 0);
				PromiseRaw* nextSibling = next->m_sibling;
				switch (process->m_state)
				{
				case PromiseRaw::Resolved: next->onThen(process); break;
				case PromiseRaw::Rejected: next->onKatch(process); break;
				case PromiseRaw::Pending:
				default:
					_assert(!"Pending promise processed\n");
					break;
				}
				next = nextSibling;
			}
			if (process->m_ending)
			{
				delete process;
			}
			process = processNext;
		} while (process);
	}
}
size_t PromisePump::count() noexcept
{
	size_t count = 0;
	PromiseRaw* prom = m_process;
	while (prom)
	{
		PromiseRaw* sib = prom;
		do
		{
			_assert((uintptr_t)sib % alignof(PromiseRaw) == 0);
			count++;
			sib = sib->m_sibling;
		} while (sib);
		prom = prom->m_next;
	}
	return count;
}
