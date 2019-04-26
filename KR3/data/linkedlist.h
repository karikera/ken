#pragma once

#include "../main.h"

#include "iterator.h"
#include "bin_search.h"

namespace kr
{
	namespace _pri_
	{
		class ChainData;
		class LinkedListData;

		template <typename Base, typename Node>
		class ChainMethod;
		template <typename Base, typename Node>
		class LinkedListMethod;
		template <typename C, bool isCRTP, bool isclass>
		class NodeImpl;
	}

	template <typename Iterator, typename T>
	class CastIterator:public Iterator
	{
	public:
		CastIterator(const Iterator & iter) noexcept
			:Iterator(iter)
		{
		}
		CastIterator(Iterator && iter) noexcept
			:Iterator(move(iter))
		{
		}
		T& operator *() noexcept
		{
			return *static_cast<T*>(&**static_cast<Iterator*>(this));
		}

		using Iterator::operator ++;
		using Iterator::operator --;
	};

	template <typename Iterable, typename T>
	class CastIterable
	{
	private:
		const Iterable * m_iterable;

	public:
		using Iterator = decltype(((Iterable*)0)->begin());
		CastIterable(const Iterable * iterable) noexcept
			:m_iterable(iterable)
		{
		}

		CastIterator<Iterator, T> begin() const noexcept
		{
			return m_iterable->begin();
		}
		CastIterator<Iterator, T> end() const noexcept
		{
			return m_iterable->end();
		}
	};

	class NodeLinkData
	{
		friend _pri_::ChainData;
		template <typename Base, typename Node>
		friend class _pri_::ChainMethod;
		template <typename Base, typename Node>
		friend class _pri_::LinkedListMethod;
	public:
		NodeLinkData() noexcept;
		NodeLinkData * getNext() noexcept;
		NodeLinkData * getPrevious() noexcept;

	protected:
		NodeLinkData* m_next;
		NodeLinkData* m_previous;

		void _detach() noexcept;
		void _insertAfter(NodeLinkData* node) noexcept;
		void _insertBefore(NodeLinkData* node) noexcept;
		template <typename Node>
		size_t _cut() noexcept
		{
			NodeLinkData *t = this;
			size_t len = 0;
			do
			{
				NodeLinkData *s = t;
				t = s->m_next;
				delete static_cast<Node*>(s);
				len++;
			} while (t != nullptr);
			return len;
		}
	};

	template <typename C>
	class _pri_::NodeImpl<C, false, true>:
		public C, public NodeLinkData
	{
	public:
		NodeImpl() = default;
		NodeImpl(const C & _copy)
			:C(_copy)
		{
		}
		NodeImpl(C && _move)
			:C((C&&)_move)
		{
		}
		using C::C;
		Node<C> * getNext() noexcept
		{
			return static_cast<Node<C>*>(m_next);
		}
		Node<C> * getPrevious() noexcept
		{
			return static_cast<Node<C>*>(m_previous);
		}
		C& data() noexcept
		{
			return *static_cast<C*>(this);
		}
		const C& data() const noexcept
		{
			return *static_cast<const C*>(this);
		}

	};

	template <typename C>
	class _pri_::NodeImpl<C, true, true> :
		public NodeLinkData
	{
	public:
		NodeImpl() = default;
		C * getNext() noexcept
		{
			return static_cast<C*>(m_next);
		}
		C * getPrevious() noexcept
		{
			return static_cast<C*>(m_previous);
		}
		C& data() noexcept
		{
			return *static_cast<C*>(this);
		}
		const C& data() const noexcept
		{
			return *static_cast<const C*>(this);
		}
		operator C&() noexcept
		{
			return *static_cast<C*>(this);
		}
		operator const C&() const noexcept
		{
			return *static_cast<const C*>(this);
		}

	};

	template <typename C>
	class _pri_::NodeImpl<C, false, false>:public NodeLinkData
	{
	public:
		NodeImpl() = default;
		NodeImpl(const C & _copy)
			:m_data(_copy)
		{
		}
		NodeImpl(C && _move)
			:m_data(move(_move))
		{
		}
		Node<C> * getNext() noexcept
		{
			return static_cast<Node<C>*>(m_next);
		}
		Node<C> * getPrevious() noexcept
		{
			return static_cast<Node<C>*>(m_previous);
		}
		C& data() noexcept
		{
			return m_data;
		}
		const C& data() const noexcept
		{
			return m_data;
		}
		operator C&() noexcept
		{
			return m_data;
		}
		operator const C&() const noexcept
		{
			return m_data;
		}

	private:
		C m_data;
	};

	template <typename C, bool isCRTP> class Node :
		public _pri_::NodeImpl<C, isCRTP, std::is_class<C>::value>
	{
		template <typename Base, typename C2>
		friend class _pri_::ChainMethod;
		using Super = _pri_::NodeImpl<C, isCRTP, std::is_class<C>::value>;

	protected:
		using Super::m_next;
		using Super::m_previous;

	public:
		using Super::Super;

	private:
		using NodeLinkData::_detach;

		void _insertAfter(Node* node) noexcept
		{
			Super::_insertAfter(node);
		}
		void _insertBefore(Node* node) noexcept
		{
			Super::_insertBefore(node);
		}
		size_t _cut() noexcept
		{
			return Super::template _cut<Node>();
		}

	};;

	template <typename T>
	struct Key<Node<T>>:Key<T>{};

	namespace _pri_
	{
		class ChainData
		{
		public:
			class Iterator :public TIterator<Iterator, NodeLinkData>
			{
			public:
				INHERIT_ITERATOR(TIterator<Iterator, NodeLinkData>);
				Iterator& operator ++() noexcept;
				Iterator& operator --() noexcept;

			protected:
				using Super::m_pt;
			};
			using ReverseIterator = Iterator::Reverse;

			ChainData() noexcept;
			~ChainData() noexcept;

			size_t calculateCount() noexcept;
			void reaxis(NodeLinkData* node) noexcept;
			void attach(NodeLinkData* node) noexcept;
			void attachFirst(NodeLinkData* node) noexcept;
			void attachAfter(NodeLinkData* node, NodeLinkData* axis) noexcept;
			void attachBefore(NodeLinkData* node, NodeLinkData* axis) noexcept;
			void attachMoveAll(ChainData* chain) noexcept;
			void attachMoveAll(LinkedListData* link) noexcept;
			bool empty() const noexcept;

			void detach(NodeLinkData* node) noexcept;
			Iterator detach(Iterator _node) noexcept;
			void detachAll() noexcept;
			NodeLinkData* detachFirst() noexcept;
			NodeLinkData* detachLast() noexcept;
			void pop() noexcept;
			void clear() noexcept;

			NodeLinkData * getNode(size_t n) noexcept;
			const NodeLinkData * getNode(size_t n) const noexcept;

			Iterator begin() const noexcept;
			Iterator end() const noexcept;
			typename Iterator::Reverse rbegin() const noexcept;
			typename Iterator::Reverse rend() const noexcept;

			template <typename T>
			CastIterable<ChainData, T> as() noexcept
			{
				return this;
			}

			bool operator ==(const ChainData& node) const noexcept;
			bool operator !=(const ChainData& node) const noexcept;

		protected:
			void _moveList(ChainData & data) noexcept;

			NodeLinkData m_axis;
		};

		class LinkedListData :public ChainData
		{
			friend ChainData;
		public:
			LinkedListData() noexcept;
			void attach(NodeLinkData* node) noexcept;
			void attachFirst(NodeLinkData * node) noexcept;
			void attachAfter(NodeLinkData* node, NodeLinkData* axis) noexcept;
			void attachBefore(NodeLinkData* node, NodeLinkData* axis) noexcept;
			void attachMoveAll(ChainData* chain) noexcept;
			void attachMoveAll(LinkedListData* link) noexcept;

			NodeLinkData * detachFirst() noexcept;
			NodeLinkData * detachLast() noexcept;

			void detach(NodeLinkData * node) noexcept;
			Iterator detach(Iterator node) noexcept;
			void detachAll() noexcept;
			void clear() noexcept;
			void check() noexcept;

			size_t size() const noexcept;

		protected:
			void _moveList(LinkedListData & data) noexcept;

			size_t m_size;
		};

		template <typename Base, typename Node>
		class ChainMethod:public Base
		{
			// static_assert(is_base_of_v<NodeLinkData, Node>, "Node must inherit NodeLinkData"); // error when node contains linkedlist
			using Super = Base;
		protected:
			using Super::m_axis;


			template <typename F> class Sorter
			{
			public:
				Sorter(F &func, NodeLinkData * axis) noexcept
					:m_cmpfunc(move(func)), m_list(axis->m_next)
				{
					NodeLinkData * last = axis->m_previous;
					last->m_next = nullptr;
					m_list->m_previous = last;
				}
				NodeLinkData* sort() noexcept
				{
					NodeLinkData* p;
					try
					{
						p = _pop();
					}
					catch (EofException&)
					{
						return nullptr;
					}
					try
					{
						int level = 0;
						for (;;)
						{
							p = _merge(p, _pickLevel(level++));
						}
					}
					catch (EofException&)
					{
						return p;
					}
				}

			private:
				F &m_cmpfunc;
				NodeLinkData* m_list;

				NodeLinkData* _pop() // EofException
				{
					NodeLinkData*  p = m_list;
					if (p == nullptr) throw EofException();
					m_list = p->m_next;
					if (m_list != nullptr)
					{
						m_list->m_previous = p->m_previous;
					}
					p->m_next = nullptr;
					p->m_previous = p;
					return p;
				}
				NodeLinkData* _pickLevel(uint level) // EofException
				{
					if (level == 0)
					{
						return _pop();
					}
					else
					{
						level--;
						NodeLinkData *a, *b;
						a = _pickLevel(level);
						try { b = _pickLevel(level); }
						catch (EofException&) { return a; }
						return _merge(a, b);
					}
				}
				// 첫번째 노드의 pPrevious에 마지막 노드를 넣는다.
				// 마지막 노드의 pNext에는 nullptr을 넣는다.
				NodeLinkData* _merge(NodeLinkData *a, NodeLinkData *b) noexcept
				{
					NodeLinkData * pOut;
					NodeLinkData * pLast;
					NodeLinkData * alast = a->getPrevious();
					NodeLinkData * blast = b->getPrevious();

					if (m_cmpfunc(static_cast<Node*>(a), static_cast<Node*>(b)) > 0)
					{
						pOut = pLast = b;
						b = b->getNext();
						if (b == nullptr)
						{
							goto _connect;
						}
					}
					else
					{
						pOut = pLast = a;
						a = a->getNext();
						if (a == nullptr)
						{
							a = b;
							alast = blast;
							goto _connect;
						}
					}

					for (;;)
					{
						if (m_cmpfunc(static_cast<Node*>(a), static_cast<Node*>(b)) > 0)
						{
							pLast->m_next = b;
							b->m_previous = pLast;
							pLast = b;
							b = b->getNext();
							if (b == nullptr) break;
						}
						else
						{
							pLast->m_next = a;
							a->m_previous = pLast;
							pLast = a;
							a = a->getNext();
							if (a == nullptr)
							{
								a = b;
								alast = blast;
								break;
							}
						}
					}

				_connect:
					pLast->m_next = a;
					pOut->m_previous = alast;
					a->m_previous = pLast;
					return pOut;
				}
			};

		public:
			using Super::empty;

			class Iterator :public TIterator<Iterator, Node>
			{
			public:
				INHERIT_ITERATOR(TIterator<Iterator, Node>);

			protected:
				using Super::m_pt;

			public:
				Iterator& operator ++() noexcept
				{
					m_pt = static_cast<Node*>(m_pt->getNext());
					return *this;
				}
				Iterator& operator --() noexcept
				{
					m_pt = static_cast<Node*>(m_pt->getPrevious());
					return *this;
				}
			};

			ChainMethod() noexcept
			{
			}
			~ChainMethod() noexcept
			{
				NodeLinkData* p = m_axis.m_next;
				while (p != &m_axis)
				{
					NodeLinkData* t = p;
					p = p->m_next;
					delete static_cast<Node*>(t);
				}
			}

			template <typename ... ARGS> Node* create(ARGS && ... args) noexcept
			{
				Node * node = _new Node(forward<ARGS>(args) ...);
				attach(node);
				return node;
			}
			template <typename ... ARGS> Node* createFirst(ARGS && ... args) noexcept
			{
				Node * node = _new Node(forward<ARGS>(args) ...);
				attachFirst(node);
				return node;
			}
			template <typename ... ARGS> Node* createAfter(Node* axis, ARGS && ... args) noexcept
			{
				Node * node = _new Node(forward<ARGS>(args) ...);
				attachAfter(node, axis);
				return node;
			}
			template <typename ... ARGS> Node* createBefore(Node* axis, ARGS && ... args) noexcept
			{
				Node * node = _new Node(forward<ARGS>(args) ...);
				attachBefore(node, axis);
				return node;
			}
			void reaxis(Node* node) noexcept
			{
				Super::reaxis(node);
			}
			void attach(Node* node) noexcept
			{
				Super::attach(node);
			}
			void attachFirst(Node* node) noexcept
			{
				Super::attachFirst(node);
			}
			void attachAfter(Node* node, Node* axis) noexcept
			{
				Super::attachAfter(node, axis);
			}
			void attachBefore(Node* node, Node* axis) noexcept
			{
				Super::attachBefore(node, axis);
			}
			void attachMoveAll(Chain<Node>* chain) noexcept
			{
				Super::attachMoveAll(chain);
			}
			void attachMoveAll(LinkedList<Node>* link) noexcept
			{
				Super::attachMoveAll(link);
			}

			void detach(Node* node) noexcept
			{
				Super::detach(node);
			}
			Iterator detach(Iterator _node) noexcept
			{
				Iterator ret = _node;
				++ret;
				detach((Node*)_node);
				return ret;
			}
			void detachAll() noexcept
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
			Node* detachFirst() noexcept
			{
				_assert(!empty());
				Node * node = static_cast<Node*>(m_axis.m_next);
				node->_detach();
				return node;
			}
			Node* detachLast() noexcept
			{
				_assert(!empty());
				Node * node = static_cast<Node*>(m_axis.m_previous);
				node->_detach();
				return node;
			}
			void remove(Node* node) noexcept
			{
				detach(node);
				delete node;
			}
			Iterator remove(Iterator _node) noexcept
			{
				Iterator ret = _node;
				++ret;
				remove((Node*)_node);
				return ret;
			}
			typename Iterator::Reverse remove(typename Iterator::Reverse _node) noexcept
			{
				typename Iterator::Reverse ret = _node;
				++ret;
				remove((Node*)_node);
				return ret;
			}
			void removeFirst() noexcept
			{
				delete detachFirst();
			}
			void pop() noexcept
			{
				delete detachLast();
			}
			void clear() noexcept
			{
				NodeLinkData* p = m_axis.m_next;
				while (p != &m_axis)
				{
					NodeLinkData* t = p;
					p = p->m_next;
					delete static_cast<Node*>(t);
				}
				m_axis.m_previous = m_axis.m_next = &m_axis;
			}

			Node * get(size_t n) noexcept
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
				return static_cast<Node*>(p);
			}
			const Node * get(size_t n) const noexcept
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
				return static_cast<const Node*>(p);
			}

			Node* front() noexcept
			{
				return static_cast<Node*>(m_axis.m_next);
			}
			Node* back() noexcept
			{
				return static_cast<Node*>(m_axis.m_previous);
			}
			const Node* front() const noexcept
			{
				return static_cast<Node*>(m_axis.m_next);
			}
			const Node* back() const noexcept
			{
				return static_cast<Node*>(m_axis.m_previous);
			}

			void sortAttach(Node* _node) noexcept
			{
				using KeyType = decltype(Key<Node>::getKey(*(Node*)0));

				KeyType insertKey = Key<Node>::getKey(*_node);

				NodeLinkData * p = m_axis.m_next;
				for (; p != &m_axis; p = p->m_next)
				{
					KeyType compareKey = Key<Node>::getKey(*static_cast<Node*>(p));
					if (Comparator<Key<Node>>::compare(insertKey, compareKey) > 0)
						continue;
					p->_insertBefore(_node);
					return;
				}

				attach(_node);
			}
			size_t cut(Node* axis) noexcept
			{
				m_axis.m_previous->m_next = nullptr;
				m_axis.m_previous = axis->m_previous;
				if (!empty()) m_axis.m_previous->m_next = &m_axis;
				return m_axis.m_previous->m_next->template _cut<Node>();
			}
			size_t cutR(Node* axis) noexcept
			{
				m_axis.m_next = axis->m_next;
				if (!empty()) m_axis.m_next->m_previous = &m_axis;

				NodeLinkData *t = axis->m_previous;

				size_t len = 0;
				while (t)
				{
					NodeLinkData* s = t;
					t = s->m_previous;
					delete static_cast<Node*>(s);
					len++;
				}
				return len;
			}
			template <typename LAMBDA>
			bool removeMatch(LAMBDA & lambda) throws(...)
			{
				Iterator iter_end = end();
				Iterator iter = begin();
				while (iter != iter_end)
				{
					Node * node = iter;
					if (lambda(node))
					{
						remove(node);
						return true;
					}
					iter++;
				}
				return false;
			}
			template <typename LAMBDA>
			size_t removeMatchAll(const LAMBDA & lambda) throws(...)
			{
				size_t count = 0;
				Iterator iter_end = end();
				Iterator iter = begin();
				while (iter != iter_end)
				{
					Node * node = iter;
					iter++;
					if (lambda(node))
					{
						remove(node);
						count++;
					}
				}
				return count;
			}
			template <typename LAMBDA> 
			void forEachForRemove(const LAMBDA & lambda) throws(...)
			{
				Iterator iter_end = end();
				Iterator iter = begin();
				while (iter != iter_end)
				{
					Node * node = iter;
					iter++;
					lambda(node);
				}
			}
			template <typename F>
			void sort(F &cmpfunc) noexcept
			{
				Sorter<F> sort(cmpfunc, &m_axis);
				NodeLinkData * first = sort.sort();
				NodeLinkData * last = first->m_previous;
				m_axis.m_next = first;
				m_axis.m_previous = last;
				first->m_previous = last->m_next = static_cast<Node*>(&m_axis);
			}
			template <typename F, typename V>
			Node* search(F cmpfunc, V value) noexcept
			{
				Node* p = m_axis.m_next;
				while (p != &m_axis)
				{
					if (cmpfunc(p, value)) return p;
					p = p->m_next;
				}
				return nullptr;
			}

			ReverseIterable<ChainMethod> reverse() noexcept
			{
				return ReverseIterable<ChainMethod>(this);
			}
			Iterator begin() const noexcept
			{
				return static_cast<Node*>(m_axis.m_next);
			}
			Iterator end() const noexcept
			{
				return static_cast<Node*>(const_cast<NodeLinkData*>(&m_axis));
			}
			typename Iterator::Reverse rbegin() noexcept
			{
				return static_cast<Node*>(m_axis.m_previous);
			}
			typename Iterator::Reverse rend() noexcept
			{
				return static_cast<Node*>(&m_axis);
			}

			template <typename T>
			CastIterable<ChainMethod, T> as() noexcept
			{
				return this;
			}
		};
		
		template <typename Base, typename Node>
		class LinkedListMethod:public ChainMethod<Base, Node>
		{
			using Super = ChainMethod<Base, Node>;

		private:
			using Super::calculateCount;

		protected:
			using Super::m_size;
			using Super::m_axis;

		public:
			using typename Super::Iterator;
			using ReverseIterator = typename Super::Iterator::Reverse;

			using Super::getNode;
			using Super::operator ==;
			using Super::operator !=;
			using Super::empty;
			using Super::size;
			using Super::begin;
			using Super::end;
			using Super::rbegin;
			using Super::rend;
			using Super::reverse;
			using Super::cutR;
			using Super::cut;

			void sortAttach(NodeLinkData* _node) noexcept
			{
				m_size++;
				return Super::sortAttach(_node);
			}
			template <typename LAMBDA> 
			bool removeMatch(const LAMBDA & lambda) throws(...)
			{
				return Super::removeMatch(lambda);
			}
			template <typename LAMBDA> 
			size_t removeMatchAll(const LAMBDA & lambda) throws(...)
			{
				return Super::removeMatchAll(lambda);
			}
			void clear() noexcept
			{
				m_size = 0;
				Super::clear();
			}
			void resize(size_t length) noexcept
			{
				if (m_size <= length) return;
				m_size -= cutR(getNode(length));
			}
			void resizeR(size_t length) noexcept
			{
				if (m_size <= length) return;
				m_size -= cut(getNode(m_size - length));
			}

			template <typename S>
			void serialize(S & s) throws(...)
			{
				size_t sz;

				if (s.read)
				{
					clear();
					s.leb128(sz);
					for (size_t i = 0; i < sz; i++)
					{
						Node * node = _new Node;
						s & *node;
						attach(node);
					}
				}
				if (s.write)
				{
					sz = size();
					s.leb128(sz);
					for (Node & node : *this)
					{
						s & node;
					}
				}
			}
		};
	}

	// 길이를 가지지 않은 링크드 리스트이다.
	template <typename Node> 
	class Chain:public TIterable<Chain<Node>, _pri_::ChainMethod<_pri_::ChainData, Node>>
	{
		using Super = TIterable<Chain<Node>, _pri_::ChainMethod<_pri_::ChainData, Node>>;
	protected:
		using Super::_moveList;

	public:
		using Super::operator =;
		using Super::operator ==;
		using Super::operator !=;

		Chain() noexcept
		{
		}
		Chain(Chain && _move) noexcept
		{
			_moveList(_move);
		}
		Chain & operator =(Chain && _move) noexcept
		{
			this->~Chain();
			_moveList(_move);
			return *this;
		}
	};

	template <typename Node> 
	class LinkedList:public _pri_::LinkedListMethod<_pri_::LinkedListData, Node>
	{
		using Super = _pri_::LinkedListMethod<_pri_::LinkedListData, Node>;
	protected:
		using Super::_moveList;

	public:
		using Super::operator ==;
		using Super::operator !=;

		LinkedList() noexcept
		{
		}
		LinkedList(LinkedList && _move) noexcept
		{
			_moveList(_move);
		}
		LinkedList & operator =(LinkedList && _move) noexcept
		{
			this->~LinkedList();
			_moveList(_move);
			return *this;
		}
	};

}

extern template class kr::LinkedList<kr::Node<int>>;
