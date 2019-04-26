#include "stdafx.h"
#include "binarray.h"

#include "../meta/log2.h"

kr::BinArray::BinArray() noexcept
{
	m_map = nullptr;
}
kr::BinArray::BinArray(size_t size) noexcept
{
	size_t uintsize = (size + bitsize - 1) >> bitshift;
	uintptr_t * data = _new uintptr_t[uintsize + 1];
	*data++ = size;
	m_map = data;
}
kr::BinArray::~BinArray() noexcept
{
	if (m_map != nullptr)
		delete[] (m_map-1);
}
void kr::BinArray::alloc(size_t size) noexcept
{
	this->~BinArray();
	new(this) BinArray(size);
}
uintptr_t * kr::BinArray::data() noexcept
{
	return m_map;
}
const uintptr_t * kr::BinArray::data() const noexcept
{
	return m_map;
}
size_t kr::BinArray::size() const noexcept
{
	if (m_map == nullptr)
		return 0;
	return m_map[-1];
}
size_t kr::BinArray::sizep() const noexcept
{
	return (size() + bitsize - 1) >> bitshift;
}
void kr::BinArray::resizeAsFalse(size_t to) noexcept
{
	size_t fromp = sizep();
	size_t top = (to + bitsize - 1) / bitsize;
	uintptr_t * dest = _new uintptr_t[top +1];
	*dest++ = to;
	memcpy(dest, m_map, fromp * sizeof(uintptr_t));
	memset(dest + fromp, 0, (top - fromp) * sizeof(uintptr_t));
	if(fromp != 0)
		dest[fromp-1] &= (1 << (size() % bitsize)) -1;
	if(m_map != nullptr)
		delete[] (m_map-1);
	m_map = dest;
}
void kr::BinArray::free() noexcept
{
	if (m_map != nullptr)
		delete[] (m_map-1);
	m_map = nullptr;
}
