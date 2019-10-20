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
		// m_buffer�� ���� offset�̴�.
		// ������ ��Ʈ�� �ܾ��� ���� �ǹ��Ѵ�.
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

	// ���Ϳ� ���ڿ� �߰�
	ChatFilter& operator <<(Text16 text) noexcept;

	// ���͵� ���ڿ� ���� ��������
	Array<Range> check(Text16 text, size_t offset = 0) const noexcept;

	// ���͵� ���ڿ� ���� ��������
	Array<Range> check(Text16 text, Context * ctx, size_t offset = 0) const noexcept;

private:
	// Node::children���� index ����ũ
	static constexpr int CHILD_MASK = ((uint)-1 >> 1);

	// Node::children���� ����� ���� �ǹ��ϴ� �ε��� ��
	static constexpr int CHILD_END = CHILD_MASK;

	// Node::children���� �ܾ��� ���� �ǹ��ϴ� ��Ʈ
	// �ܾ��� ��������, ����� ���� �ƴ� �� �ִ�. (��: '��'�� ���̰�, '����'�� ���̴�. )
	static constexpr int FINISH_NODE = -1 ^ CHILD_MASK;

	Array<Node> m_buffer;

	// �ڽ� ��� ����
	Node* _makeChild(Node * node, size_t index) noexcept;

	const Node* _check(Array<Range> *out, Text16 text, const Node * startNode, size_t offset) const noexcept;
};