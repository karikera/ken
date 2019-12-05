#include "stdafx.h"
#include "filter.h"

#include <unordered_set>

kr::ChatFilter::Context::Context() noexcept
{
	m_previousNode = nullptr;
}

kr::ChatFilter::ChatFilter() noexcept
{
	m_buffer.resize(16);
	mem32::set(m_buffer.begin(), CHILD_END, 16);
}

kr::ChatFilter::ChatFilter(initializer_list<Text16> texts) noexcept
	:ChatFilter()
{
	for (Text16 tx : texts)
	{
		*this << tx;
	}
}

kr::ChatFilter& kr::ChatFilter::operator <<(Text16 text) noexcept
{
	Node firstNode = { 0 };
	Node * node = &firstNode;
	for (const char16 &chr : text)
	{
		node = _makeChild(node, chr & 0xf);
		node = _makeChild(node, (chr >> 4) & 0xf);

		if ((chr >> 8) != 0)
		{
			node = _makeChild(node, (chr >> 8) & 0xf);
			node = _makeChild(node, chr >> 12);
		}
	}
	node->children |= FINISH_NODE;
	return *this;
}

kr::Array<kr::Range> kr::ChatFilter::check(Text16 text, size_t offset) const noexcept
{
	Array<Range> range;
	range.reserve(8);
	_check(&range, text, nullptr, offset);
	return range;
}

kr::Array<kr::Range> kr::ChatFilter::check(Text16 text, Context * ctx, size_t offset) const noexcept
{
	Array<Range> range;
	range.reserve(8);
	ctx->m_previousNode = const_cast<Node*>(_check(&range, text, ctx->m_previousNode, offset));
	return range;
}

kr::ChatFilter::Node * kr::ChatFilter::_makeChild(Node * node, size_t index) noexcept
{
	if ((node->children & CHILD_MASK) != CHILD_END)
		return (Node*)m_buffer.begin() + (node->children & CHILD_MASK) + index;
	size_t childoffset = m_buffer.size();
	node->children &= childoffset | FINISH_NODE;
	Node* nodes = (Node*)m_buffer.prepare(16);
	mem32::set(nodes, CHILD_END, 16);
	return (Node*)m_buffer.begin() + childoffset + index;
}

struct Hangle
{
	int first, middle, last;
	static constexpr int LAST_COUNT = 28;
	static constexpr int MIDDLE_COUNT = 21;
	static constexpr int FIRST_COUNT = 19;

	Hangle() = default;
	Hangle(char16 chr) noexcept;
};

Hangle::Hangle(char16 chr) noexcept
{
	_assert(u'가' <= chr && chr <= u'힣');
	int v = chr - u'가';
	last = v % LAST_COUNT;
	v /= LAST_COUNT;
	middle = v % MIDDLE_COUNT;
	v /= MIDDLE_COUNT;
	first = v;
}

const kr::ChatFilter::Node* kr::ChatFilter::_check(Array<Range> *out, Text16 text, const Node * startNode, size_t offset) const noexcept
{
	static const std::unordered_set<char> _list = {
		'~', '!', '@', '#', '$',
		'%', '^', '&', '*', '(',
		')', '_', '+', '`', '\\',
		'\\', '\'', '\"', ':', ';',
		'[', '{', ']', '}', ',',
		'<', '.', '>', '/', '?',
		'-', '=', ' ', '\t',
	};


	const Node firstNode = { 0 };
	size_t start = offset;
	size_t nextSearch = offset;
	const Node * nodebeg = m_buffer.begin();

	auto mapend = _list.end();
	if (startNode == nullptr)
	{
		startNode = &firstNode;
		nextSearch++;
	}
	const Node * node = startNode;

	size_t size = text.size();
	size_t prevEnd = 0;
	char16 previous = 0;
	for (size_t i = offset; i < size; i++)
	{
	_retry:
		char16 chr = text[i];
		if ('A' <= chr && chr <= 'Z')
			chr += 'a' - 'A';

		if (start != i)
		{
			if (_list.find((char)chr) != mapend)
			{
				continue;
			}
			if (!('a' <= previous && previous <= 'z'))
			{
				if (u'a' <= chr && chr <= 'z')
				{
					continue;
				}
			}
			if ((u'0' <= chr && chr <= '9') ||
				(u'ㄱ' <= chr && chr <= u'ㅎ') ||
				(u'ㅏ' <= chr && chr <= u'ㅣ'))
			{	
				continue;
			}

			if (u'가' <= chr && chr <= u'힣' &&
				u'가' <= previous && previous <= u'힣')
			{
				Hangle hangle(chr);
				if (hangle.first == Hangle(u'아').first)
				{
					if (hangle.middle == Hangle(previous).middle)
					{
						if (hangle.last == 0)
						{
							continue;
						}
					}
				}
			}
		}
		int childIndex = node->children & CHILD_MASK;
		if (childIndex == CHILD_END)
			goto _notfound;
		node = nodebeg + childIndex + (chr & 0xf);

		childIndex = node->children & CHILD_MASK;
		if (childIndex == CHILD_END)
			goto _notfound;
		node = nodebeg + childIndex + ((chr >> 4) & 0xf);

		if ((chr >> 8) != 0)
		{
			childIndex = node->children & CHILD_MASK;
			if (node->children == CHILD_END)
				goto _notfound;
			node = nodebeg + childIndex + ((chr >> 8) & 0xf);

			childIndex = node->children & CHILD_MASK;
			if (node->children == CHILD_END)
				goto _notfound;
			node = nodebeg + childIndex + (chr >> 12);
		}

		previous = chr;
		if (node->children < 0)
			prevEnd = i + 1;
		continue;
	_notfound:
		if (prevEnd != 0)
		{
			out->push({ start , prevEnd });
			nextSearch = start = prevEnd;
			prevEnd = 0;
		}
		i = nextSearch;
		start = nextSearch;
		nextSearch++;
		node = &firstNode;
		if (i >= size) break;
		goto _retry;
	}

	if (prevEnd != 0)
		out->push({ start , prevEnd });
	return node;
}
