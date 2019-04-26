#include "stdafx.h"
#include "linkedlist.h"

using namespace kr;

NodeLinkData::NodeLinkData() noexcept
{
	m_next = nullptr;
	m_previous = nullptr;
}
NodeLinkData * NodeLinkData::getNext() noexcept
{
	return m_next;
}
NodeLinkData * NodeLinkData::getPrevious() noexcept
{
	return m_previous;
}
void NodeLinkData::_detach() noexcept
{
	m_previous->m_next = m_next;
	m_next->m_previous = m_previous;
	m_previous = nullptr;
	m_next = nullptr;
}
void NodeLinkData::_insertAfter(NodeLinkData* node) noexcept
{
	if (m_next != nullptr)
	{
		m_next->m_previous = node;
		node->m_next = m_next;
	}
	m_next = node;
	node->m_previous = this;
}
void NodeLinkData::_insertBefore(NodeLinkData* node) noexcept
{
	if (m_previous != nullptr)
	{
		m_previous->m_next = node;
		node->m_previous = m_previous;
	}
	m_previous = node;
	node->m_next = this;
}

_pri_::ChainData::Iterator& _pri_::ChainData::Iterator::operator ++() noexcept
{
	m_pt = m_pt->getNext();
	return *this;
}
_pri_::ChainData::Iterator& _pri_::ChainData::Iterator::operator --() noexcept
{
	m_pt = m_pt->getPrevious();
	return *this;
}

_pri_::ChainData::ChainData() noexcept
{
	m_axis.m_previous = m_axis.m_next = &m_axis;
}
_pri_::ChainData::~ChainData() noexcept
{
}

size_t _pri_::ChainData::calculateCount() noexcept
{
	size_t count = 0;
	NodeLinkData * p = m_axis.m_next;
	while (p != &m_axis)
	{
		_assert(p->m_next->m_previous == p);
		p = p->m_next;
		count++;
	}
	return count;
}
void _pri_::ChainData::reaxis(NodeLinkData* node) noexcept
{
	m_axis._detach();

	NodeLinkData * pnode = node->m_previous;
	m_axis.m_previous = pnode;
	pnode->m_next = &m_axis;
	m_axis.m_next = node;
	node->m_previous = &m_axis;
}
void _pri_::ChainData::attach(NodeLinkData* node) noexcept
{
	m_axis.m_previous->m_next = node;
	node->m_next = &m_axis;
	node->m_previous = m_axis.m_previous;
	m_axis.m_previous = node;
}
void _pri_::ChainData::attachFirst(NodeLinkData* node) noexcept
{
	if (m_axis.m_previous == &m_axis)
	{
		attach(node);
	}
	else
	{
		m_axis.m_next->m_previous = node;
		node->m_next = m_axis.m_next;
		m_axis.m_next = node;
		node->m_previous = &m_axis;
	}
}
void _pri_::ChainData::attachAfter(NodeLinkData* node, NodeLinkData* axis) noexcept
{
	axis->_insertAfter(node);
}
void _pri_::ChainData::attachBefore(NodeLinkData* node, NodeLinkData* axis) noexcept
{
	axis->_insertBefore(node);
}
void _pri_::ChainData::attachMoveAll(ChainData* chain) noexcept
{
	if (chain->empty()) return;

	NodeLinkData * first = chain->m_axis.m_next;
	first->m_previous = m_axis.m_previous;
	m_axis.m_previous->m_next = first;
	NodeLinkData * last = chain->m_axis.m_previous;
	last->m_next = &m_axis;
	m_axis.m_previous = last;

	chain->m_axis.m_previous = chain->m_axis.m_next = &chain->m_axis;
}
void _pri_::ChainData::attachMoveAll(LinkedListData* link) noexcept
{
	attachMoveAll(static_cast<ChainData*>(link));
	link->m_size = 0;
}
bool _pri_::ChainData::empty() const noexcept
{
	return m_axis.m_next == &m_axis;
}

void _pri_::ChainData::detach(NodeLinkData* node) noexcept
{
	node->_detach();
}
_pri_::ChainData::Iterator _pri_::ChainData::detach(Iterator _node) noexcept
{
	Iterator ret = _node;
	++ret;
	detach((NodeLinkData*)_node);
	return ret;
}
void _pri_::ChainData::detachAll() noexcept
{
	NodeLinkData* p = m_axis.m_next;
	while (p != &m_axis)
	{
		NodeLinkData* t = p;
		p = p->m_next;
		t->m_previous = nullptr;
		t->m_next = nullptr;
	}
	m_axis.m_previous = m_axis.m_next = &m_axis;
}
NodeLinkData* _pri_::ChainData::detachFirst() noexcept
{
	_assert(!empty());
	NodeLinkData * node = m_axis.m_next;
	node->_detach();
	return node;
}
NodeLinkData* _pri_::ChainData::detachLast() noexcept
{
	_assert(!empty());
	NodeLinkData * node = m_axis.m_previous;
	node->_detach();
	return node;
}
void _pri_::ChainData::pop() noexcept
{
	delete detachLast();
}
void _pri_::ChainData::clear() noexcept
{
	NodeLinkData* p = m_axis.m_next;
	while (p != &m_axis)
	{
		NodeLinkData* t = p;
		p = p->m_next;
		delete static_cast<NodeLinkData*>(t);
	}
	m_axis.m_previous = m_axis.m_next = &m_axis;
}

NodeLinkData * _pri_::ChainData::getNode(size_t n) noexcept
{
	NodeLinkData* p = m_axis.m_next;
	if (p == &m_axis)
		return nullptr;
	while (n--)
	{
		p = p->m_next;
		if (p == &m_axis)
			return nullptr;
	}
	return static_cast<NodeLinkData*>(p);
}
const NodeLinkData * _pri_::ChainData::getNode(size_t n) const noexcept
{
	const NodeLinkData* p = m_axis.m_next;
	if (p == &m_axis)
		return nullptr;
	while (n--)
	{
		p = p->m_next;
		if (p == &m_axis)
			return nullptr;
	}
	return static_cast<const NodeLinkData*>(p);
}

_pri_::ChainData::Iterator _pri_::ChainData::begin() const noexcept
{
	return m_axis.m_next;
}
_pri_::ChainData::Iterator _pri_::ChainData::end() const noexcept
{
	return const_cast<NodeLinkData*>(&m_axis);
}
typename _pri_::ChainData::Iterator::Reverse _pri_::ChainData::rbegin() const noexcept
{
	return m_axis.m_previous;
}
typename _pri_::ChainData::Iterator::Reverse _pri_::ChainData::rend() const noexcept
{
	return const_cast<NodeLinkData*>(&m_axis);
}

bool _pri_::ChainData::operator ==(const ChainData& node) const noexcept
{
	return this == &node;
}
bool _pri_::ChainData::operator !=(const ChainData& node) const noexcept
{
	return this != &node;
}

void _pri_::ChainData::_moveList(ChainData & data) noexcept
{
	m_axis = data.m_axis;
	m_axis.m_next->m_previous = &m_axis;
	m_axis.m_previous->m_next = &m_axis;

	data.m_axis.m_previous = data.m_axis.m_next = &data.m_axis;
}

_pri_::LinkedListData::LinkedListData() noexcept
{
	m_size = 0;
}
void _pri_::LinkedListData::attach(NodeLinkData* node) noexcept
{
	m_size++;
	ChainData::attach(node);
}
void _pri_::LinkedListData::attachFirst(NodeLinkData * node) noexcept
{
	m_size++;
	ChainData::attachFirst(node);
}
void _pri_::LinkedListData::attachAfter(NodeLinkData* node, NodeLinkData* axis) noexcept
{
	m_size++;
	ChainData::attachAfter(node, axis);
}
void _pri_::LinkedListData::attachBefore(NodeLinkData* node, NodeLinkData* axis) noexcept
{
	m_size++;
	ChainData::attachBefore(node, axis);
}
void _pri_::LinkedListData::attachMoveAll(ChainData* chain) noexcept
{
	m_size += chain->calculateCount();
	ChainData::attachMoveAll(chain);
}
void _pri_::LinkedListData::attachMoveAll(LinkedListData* link) noexcept
{
	m_size += link->m_size;
	link->m_size = 0;
	ChainData::attachMoveAll(static_cast<ChainData*>(link));
}

NodeLinkData * _pri_::LinkedListData::detachFirst() noexcept
{
	m_size--;
	return ChainData::detachFirst();
}
NodeLinkData * _pri_::LinkedListData::detachLast() noexcept
{
	m_size--;
	return ChainData::detachLast();
}

void _pri_::LinkedListData::detach(NodeLinkData * node) noexcept
{
	m_size--;
	ChainData::detach(node);
}
_pri_::LinkedListData::Iterator _pri_::LinkedListData::detach(Iterator node) noexcept
{
	m_size--;
	return ChainData::detach(node);
}
void _pri_::LinkedListData::detachAll() noexcept
{
	m_size = 0;
	ChainData::detachAll();
}
void _pri_::LinkedListData::clear() noexcept
{
	m_size = 0;
	ChainData::clear();
}
void _pri_::LinkedListData::check() noexcept
{
#ifndef NDEBUG
	_assert(calculateCount() == size());
#endif
}
size_t _pri_::LinkedListData::size() const noexcept
{
	return m_size;
}

void _pri_::LinkedListData::_moveList(LinkedListData & data) noexcept
{
	ChainData::_moveList(data);
	m_size = data.m_size;
	data.m_size = 0;
}

template class kr::LinkedList<Node<int>>;
