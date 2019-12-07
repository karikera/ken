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
	EventPump * manager = EventPump::getInstance();
	*manager->m_pprocess = this;
	manager->m_pprocess = &m_sibling;
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
