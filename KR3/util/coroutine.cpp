#include "stdafx.h"
#include "coroutine.h"

extern "C" void _kr_pri_prYield(kr::Yield * yield) noexcept;
extern "C" void _kr_pri_prCallFirst(kr::Coroutine * cr, void(CT_CDECL *func)(kr::Yield &)) noexcept;
extern "C" void _kr_pri_prCall(kr::Coroutine * cr) noexcept;

void kr::Yield::operator ()() noexcept
{
	_kr_pri_prYield(this);
}
kr::Coroutine::Coroutine(void(CT_CDECL *func)(Yield &), size_t stackSize) noexcept
	: m_stackSize(stackSize)
{
	m_yield.m_esp = _new byte[stackSize] + stackSize;
	_kr_pri_prCallFirst(this, func);
}
kr::Coroutine::~Coroutine() noexcept
{
	delete[](m_yield.m_esp - m_stackSize);
}
void kr::Coroutine::call() noexcept
{
	_kr_pri_prCall(this);
}
