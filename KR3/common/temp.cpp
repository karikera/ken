#include "stdafx.h"
#include "temp.h"

#include <malloc.h>
#include <cstring>

static_assert(alignof(kr::StackAllocator::Node) == alignof(kr::StackAllocator::AllocHead), "Node alignment unmatch");

inline kr::byte * alignPointerToNext(void * p, size_t alignment) noexcept
{
	_assert(((alignment - 1) & alignment) == 0);
	alignment--;
	return (kr::byte*)(((size_t)p + alignment) & ~alignment);
}
inline kr::byte * alignPointerToPrevious(void * p, size_t alignment) noexcept
{
	_assert(((alignment - 1) & alignment) == 0);
	return (kr::byte*)((size_t)p & ~(alignment - 1));
}

kr::StackAllocator::AllocHead * kr::StackAllocator::AllocHead::get(void * p) noexcept
{
	return (AllocHead*)alignPointerToPrevious((AllocHead*)p - 1, alignof(AllocHead));
}

ATTR_NO_DISCARD kr::StackAllocator::Node* kr::StackAllocator::Node::create(Node * pnode, size_t sz) noexcept
{
	Node* newnode = (Node*)alloc<alignof(Node)>::allocate(sz);
	removeAllocDebugInfo(newnode);
	newnode->prev = pnode;
	newnode->axis.usefrom = newnode->axis.useto = (byte*)(newnode + 1);
	newnode->axis.next = &newnode->axis;
	newnode->axis.previous = &newnode->axis;
	return newnode;
}
ATTR_NO_DISCARD kr::byte * kr::StackAllocator::Node::end() noexcept
{
	return (byte*)this + kr_msize(this);
}
ATTR_NO_DISCARD size_t kr::StackAllocator::Node::size() noexcept
{
	return end() - axis.previous->useto;
}
ATTR_NO_DISCARD bool kr::StackAllocator::Node::empty() noexcept
{
	return axis.previous == &axis;
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::Node::allocate(size_t sz) noexcept
{
	AllocHead * last = axis.previous;
	AllocHead * head = (AllocHead*)alignPointerToNext(last->useto, alignof(AllocHead));
	byte * usefrom = (byte*)(head + 1);
	byte * useto = usefrom + sz;
	if (!_expandTo(useto)) return nullptr;
	return _allocate((AllocHead*)head, usefrom, useto);
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::Node::allocate(size_t sz, size_t align) noexcept
{
	if (align <= alignof(AllocHead)) return allocate(sz);
	AllocHead * last = axis.previous;
	AllocHead * head = (AllocHead*)alignPointerToNext(last->useto, alignof(AllocHead));
	byte * usefrom = alignPointerToNext(head + 1, align);
	byte * useto = usefrom + sz;
	if (!_expandTo(useto)) return nullptr;
	return _allocate(head, usefrom, useto);
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::Node::allocate(size_t sz, size_t align, size_t offset) noexcept
{
	offset &= ~(align - 1);
	if (offset == 0) return allocate(sz, align);
	AllocHead * last = axis.previous;
	AllocHead * head = (AllocHead*)alignPointerToNext(last->useto, alignof(AllocHead));
	byte * usefrom = alignPointerToNext((byte*)(head+1)+offset, align) - offset;
	byte * useto = usefrom + sz;
	if (!_expandTo(useto)) return nullptr;
	return _allocate(head, usefrom, useto);
}
void kr::StackAllocator::Node::free(void * data) noexcept
{
	AllocHead* head = AllocHead::get(data);
	
	_assert(head == axis.previous);
	// 스택에 의해 할당된게 아니거나,
	// 스택 삭제 순서가 지켜지지 않은 것으로 보인다.

	axis.previous = head->previous;
	axis.previous->next = &axis;
}
ATTR_NO_DISCARD bool kr::StackAllocator::Node::expand(void * data, size_t sz) noexcept
{
	AllocHead* head = AllocHead::get(data);
	if (head != axis.previous)
		return false;

	byte* to = (byte*)data + sz;
	if (!_expandTo(to))
		return false;
	head->useto = to;
	return true;
}
ATTR_NO_DISCARD size_t kr::StackAllocator::Node::msize(void * data) noexcept
{
	AllocHead* head = AllocHead::get(data);
	return head->useto - (byte*)data;
}
ATTR_NO_DISCARD bool kr::StackAllocator::Node::_expandTo(byte* to) noexcept
{
	if (end() >= to)
		return true;
	
	return kr_expand(this, to - (byte*)this);
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::Node::_allocate(AllocHead * head, byte * usefrom, byte * useto) noexcept
{
	head->usefrom = usefrom;
	head->useto = useto;
	head->previous = axis.previous;
	axis.previous->next = head;
	head->next = &axis;
	axis.previous = head;
	return head + 1;
}

kr::StackAllocator::StackAllocator(size_t reserve) noexcept
{
	m_fullsize = reserve;
	m_reserve = reserve;
	m_last = nullptr;
	m_lastBlock = nullptr;
}
kr::StackAllocator::~StackAllocator() noexcept
{
	_assert(empty());
	kr_free(m_last);
}
void kr::StackAllocator::terminate() noexcept
{
	Node * node = m_last;
	while (node != nullptr)
	{
		Node * prev = node->prev;
		kr_free(node);
		node = prev;
	}
	m_last = nullptr;
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::allocate(size_t sz) noexcept
{
	if (sz == 0)
		return nullptr;

	if (m_last != nullptr)
	{
		void * res = m_last->allocate(sz);
		if (res != nullptr)
		{
			return m_lastBlock = res;
		}
	}

	return allocateWithNewNode(sz);
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::allocateWithNewNode(size_t sz) noexcept
{
	size_t need = sizeof(Node) + sizeof(AllocHead) + alignof(AllocHead)-1 + sz;
	autoptr ret = _allocateNewNode(need)->allocate(sz);
	_assert(ret != nullptr);
	return m_lastBlock = ret;
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::allocate(size_t sz, size_t align) noexcept
{
	if (sz == 0)
		return nullptr;

	if (m_last != nullptr)
	{
		void * res = m_last->allocate(sz, align);
		if (res != nullptr)
		{
			return m_lastBlock = res;
		}
	}
	return allocateWithNewNode(sz, align);
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::allocateWithNewNode(size_t sz, size_t align) noexcept
{
	if (align <= alignof(Node)) return allocateWithNewNode(sz);
	size_t need = sizeof(Node) + sizeof(AllocHead) + alignof(AllocHead)-1 + sz + align - 1;
	autoptr ret = _allocateNewNode(need)->allocate(sz, align);
	_assert(ret != nullptr);
	return m_lastBlock = ret;
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::allocate(size_t sz, size_t align, size_t offset) noexcept
{
	if (sz == 0) return nullptr;

	if (m_last != nullptr)
	{
		void * res = m_last->allocate(sz, align, offset);
		if (res != nullptr)
		{
			return m_lastBlock = res;
		}
	}
	return allocateWithNewNode(sz, align, offset);
}
ATTR_NO_DISCARD kr::autoptr kr::StackAllocator::allocateWithNewNode(size_t sz, size_t align, size_t offset) noexcept
{
	offset &= ~(align - 1);
	if (offset == 0) return allocateWithNewNode(sz, align);

	size_t need = sizeof(Node) + sizeof(AllocHead) + alignof(AllocHead)-1 + sz + align + offset - 1;
	autoptr ret = _allocateNewNode(need)->allocate(sz, align, offset);
	_assert(ret != nullptr);
	return m_lastBlock = ret;
}
bool kr::StackAllocator::expand(void * data, size_t sz) noexcept
{
	if (sz == 0)
	{
		free(data);
		return true;
	}
	return m_last->expand(data, sz);
}
ATTR_NO_DISCARD size_t kr::StackAllocator::msize(void * data) noexcept
{
	return m_last->msize(data);
}
ATTR_NO_DISCARD kr::StackAllocator::Node * kr::StackAllocator::getLastNode() noexcept
{
	return m_last;
}
void kr::StackAllocator::free(void * data) noexcept
{
	if (data == nullptr) return;
	Node * last = m_last;
	_assert(last != nullptr);
	last->free(data);
	while(last->empty())
	{
		Node * n = last->prev;
		if (n == nullptr)
		{
			if(m_reserve == 0) break;
		}
		m_reserve += last->size();
		kr_free(last);
		last = n;
		if (n == nullptr) break;
	}
	m_last = last;
	if (last != nullptr)
		m_lastBlock = last->axis.previous + 1;
	else
		m_lastBlock = nullptr;
}
ATTR_NO_DISCARD bool kr::StackAllocator::empty() noexcept
{
	if(m_last == nullptr) return true;
	return m_last->empty() && m_last->prev == nullptr;
}
ATTR_NO_DISCARD bool kr::StackAllocator::isLastBlock(void * block) noexcept
{
	return m_lastBlock == block;
}

ATTR_NO_DISCARD kr::StackAllocator * kr::StackAllocator::getInstance() noexcept
{
	static thread_local kr::StackAllocator allocator;
	return &allocator;
}

ATTR_NO_DISCARD kr::StackAllocator::Node * kr::StackAllocator::_allocateNewNode(size_t need) noexcept
{
	if (need <= m_reserve)
	{
		need = m_reserve;
	}
	else
	{
		size_t inc = m_fullsize / 2;
		if (need < inc) need = inc;
		m_fullsize += need - m_reserve;
	}
	m_reserve = 0;

	return m_last = Node::create(m_last, need);
}