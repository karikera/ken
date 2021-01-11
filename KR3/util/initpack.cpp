#include "stdafx.h"
#include "initpack.h"

using namespace kr;

void InitPack::init() noexcept
{
#ifndef NDEBUG
    _assert(!m_inited);
    m_inited = true;
#endif
    kr::WView<pair<ctor_t, void*>> ctors = m_array;
    for (auto& cb : ctors)
    {
        cb.first = (ctor_t)cb.first(cb.second);
    }
    m_array.shrink();
}
void InitPack::clear() noexcept
{
#ifndef NDEBUG
    _assert(m_inited);
    m_inited = false;
#endif

    kr::WView<pair<ctor_t, void*>> dtors = m_array;
    for (auto& cb : m_array)
    {
        ((dtor_t)cb.first)(cb.second);
    }
}
void InitPack::_add(ctor_t ctor, void* param) noexcept
{
    m_array.push({ ctor, param});
}
InitPack::InitPack() noexcept
{
    ondebug(m_inited = false);
}
InitPack::~InitPack() noexcept
{
    ondebug(_assert(m_inited == false));
}