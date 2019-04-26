#include "stdafx.h"
#include "vertexlist.h"
#include "inf/miniball.h"

kr::VertexList::Iterator::Iterator() noexcept
{
}
bool kr::VertexList::Iterator::operator == (const Iterator & iter) const noexcept
{
	return iter.m_ptr == m_ptr;
}
bool kr::VertexList::Iterator::operator != (const Iterator & iter) const noexcept
{
	return iter.m_ptr != m_ptr;
}
kr::VertexList::Iterator & kr::VertexList::Iterator::operator --() noexcept
{
	m_ptr -= m_stride;
	return *this;
}
const kr::VertexList::Iterator kr::VertexList::Iterator::operator --(int) noexcept
{
	Iterator old = *this;
	m_ptr -= m_stride;
	return old;
}
kr::VertexList::Iterator & kr::VertexList::Iterator::operator ++() noexcept
{
	m_ptr += m_stride;
	return *this;
}
const kr::VertexList::Iterator kr::VertexList::Iterator::operator ++(int) noexcept
{
	Iterator old = *this;
	m_ptr += m_stride;
	return old;
}
const kr::VertexList::Iterator kr::VertexList::Iterator::operator+(std::intptr_t n) const noexcept
{
	Iterator o;
	o.m_ptr = m_ptr + n;
	o.m_stride = m_stride;
	return o;
}
const kr::VertexList::Iterator kr::VertexList::Iterator::operator-(std::intptr_t n) const noexcept
{
	Iterator o;
	o.m_ptr = m_ptr - n;
	o.m_stride = m_stride;
	return o;
}
const kr::VertexList::Iterator kr::operator+(std::intptr_t n, const kr::VertexList::Iterator & i) noexcept
{
	VertexList::Iterator o;
	o.m_ptr = i.m_ptr + n;
	o.m_stride = i.m_stride;
	return o;
}
const kr::vec3 * kr::VertexList::Iterator::operator ->() const noexcept
{
	return (vec3*)m_ptr;
}
const kr::vec3& kr::VertexList::Iterator::operator *() const noexcept
{
	return *(vec3*)m_ptr;
}
const kr::vec3& kr::VertexList::Iterator::operator [](std::uintptr_t idx) const noexcept
{
	return *(vec3*)(m_ptr + m_stride * idx);
}

kr::VertexList::VertexList(const void * start, size_t stride, size_t count) noexcept
{
	m_start = (const std::uint8_t*)start;
	m_stride = stride;
	m_count = count;
}
kr::VertexList::~VertexList() noexcept
{
}

const kr::vec3& kr::VertexList::operator[](size_t idx) const noexcept
{
	_assert(idx < m_count);
	return *(vec3*)(idx*m_stride + m_start);
}

size_t kr::VertexList::size() const noexcept
{
	return m_count;
}
kr::VertexList::Iterator kr::VertexList::begin() const noexcept
{
	Iterator iter;
	iter.m_ptr = m_start;
	iter.m_stride = m_stride;
	return iter;
}
kr::VertexList::Iterator kr::VertexList::end() const noexcept
{
	Iterator iter;
	iter.m_ptr = m_start + m_stride * m_count;
	iter.m_stride = m_stride;
	return iter;
}
kr::BoundingSphere kr::VertexList::getBoundingSphere() const noexcept
{
	Miniball ball(*this);

	BoundingSphere sphere;
	sphere.center = ball.getCenter();
	sphere.radius = sqrtf(ball.getRadiusSq());
	return sphere;
}
const void * kr::VertexList::getPointer() const noexcept
{
	return m_start;
}
size_t kr::VertexList::getStride() const noexcept
{
	return m_stride;
}

