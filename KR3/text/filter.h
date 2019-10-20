#pragma once

#include <KR3/main.h>

namespace kr
{
	class ChatFilter;
}

class kr::ChatFilter
{
private:
	struct Node
	{
		// m_buffer에 대한 offset이다.
		// 마지막 비트는 단어의 끝을 의미한다.
		int children;
	};
public:
	class Context
	{
		friend ChatFilter;
	public:
		Context() noexcept;

	private:
		Node * m_previousNode;

	};

	ChatFilter() noexcept;

	ChatFilter(initializer_list<Text16> texts) noexcept;

	// 필터에 문자열 추가
	ChatFilter& operator <<(Text16 text) noexcept;

	// 필터된 문자열 범위 가져오기
	Array<Range> check(Text16 text, size_t offset = 0) const noexcept;

	// 필터된 문자열 범위 가져오기
	Array<Range> check(Text16 text, Context * ctx, size_t offset = 0) const noexcept;

private:
	// Node::children에서 index 마스크
	static constexpr int CHILD_MASK = ((uint)-1 >> 1);

	// Node::children에서 노드의 끝을 의미하는 인덱스 값
	static constexpr int CHILD_END = CHILD_MASK;

	// Node::children에서 단어의 끝을 의미하는 비트
	// 단어의 끝이지만, 노드의 끝이 아닐 수 있다. (예: '씹'은 욕이고, '씹팔'도 욕이다. )
	static constexpr int FINISH_NODE = -1 ^ CHILD_MASK;

	Array<Node> m_buffer;

	// 자식 노드 생성
	Node* _makeChild(Node * node, size_t index) noexcept;

	const Node* _check(Array<Range> *out, Text16 text, const Node * startNode, size_t offset) const noexcept;
};