#pragma once

#define INHERIT_ITERATOR(...) \
	private: using Super = __VA_ARGS__;\
	public: using Super::Super; \
	using Super::operator *;\
	using Super::operator --;\
	using Super::operator ++;\
	using typename Super::ValueType;\
	// using Super::operator !=;\
	// using Super::operator ==;\
	// using Super::operator ->;\


namespace kr
{
	namespace _pri_
	{
		template <typename Derived, typename _ValueType, typename Parent = Empty>
		class MakeIteratorCommon:public Parent
		{
		public:
			using ValueType = _ValueType;
			using Parent::Parent;
			ValueType operator *() const noexcept
			{
				return static_cast<const Derived*>(this)->value();
			}
			Derived& operator ++()
			{
				static_cast<Derived*>(this)->next();
				return *static_cast<Derived*>(this);
			}
			Derived operator ++(int)
			{
				Derived old = *static_cast<Derived*>(this);
				static_cast<Derived*>(this)->next();
				return old;
			}
			Derived& operator --()
			{
				static_cast<Derived*>(this)->previous();
				return *static_cast<Derived*>(this);
			}
			Derived operator --(int)
			{
				Derived old = *static_cast<Derived*>(this);
				static_cast<Derived*>(this)->previous();
				return old;
			}
		};


		template <typename ValueType, typename Parent = Empty>
		class PointerIteratorData :public Parent
		{
		protected:
			ValueType* m_pt;

		public:
			PointerIteratorData(ValueType* pt) noexcept
				:m_pt(pt)
			{
			}
		};

	}
	// 이터레이터의 끝
	struct IteratorEnd
	{
		template <typename T>
		bool operator != (const T& other) const noexcept
		{
			return !other.isEnd();
		}
		template <typename T>
		bool operator == (const T& other) const noexcept
		{
			return other.isEnd();
		}
		template <typename T>
		friend bool operator != (const T & other, const IteratorEnd &) noexcept
		{
			return !other.isEnd();
		}
		template <typename T>
		friend bool operator == (const T & other, const IteratorEnd &) noexcept
		{
			return other.isEnd();
		}
	};

	template <typename Derived, typename ValueType, typename Parent = Empty>
	class MakeIterator;

	template <typename Derived, typename ValueType, typename Parent>
	class MakeIterator:public _pri_::MakeIteratorCommon<Derived, ValueType, Parent>
	{
		using Super = _pri_::MakeIteratorCommon<Derived, ValueType, Parent>;
	public:
		using Super::Super;
	};

	template <typename Derived, typename ValueType, typename Parent>
	class MakeIterator<Derived, ValueType&, Parent>:public _pri_::MakeIteratorCommon<Derived, ValueType&, Parent>
	{
		using Super = _pri_::MakeIteratorCommon<Derived, ValueType&, Parent>;
	public:
		using Super::Super;
		ValueType* operator ->() const noexcept
		{
			return &static_cast<const Derived*>(this)->value();
		}
		operator ValueType* () const noexcept
		{
			return &static_cast<const Derived*>(this)->value();
		}
	};

	template <typename Derived, typename ValueType, typename Parent = Empty>
	class MakePointerIterator:public MakeIterator<Derived, ValueType&, _pri_::PointerIteratorData<ValueType, Parent> >
	{
		using Super = MakeIterator<Derived, ValueType&, _pri_::PointerIteratorData<ValueType, Parent> >;
	protected:
		using Super::m_pt;
	public:
		using Super::Super;

		template <typename I2> bool operator ==(const MakePointerIterator& o) const noexcept
		{
			return m_pt == o.m_pt;
		}
		template <typename I2> bool operator !=(const MakePointerIterator& o) const noexcept
		{
			return m_pt != o.m_pt;
		}

		ValueType& value() const noexcept
		{
			return *m_pt;
		}

		class Reverse :public MakeIterator<Reverse, ValueType&, Derived >
		{
		public:
			INHERIT_ITERATOR(MakeIterator<Reverse, ValueType&, Derived >);

			void next()
			{
				return static_cast<Derived*>(this)->previous();
			}
			void previous()
			{
				return static_cast<Derived*>(this)->next();
			}
		};
	};


	template <class Derived, typename ValueType, typename Parent = Empty>
	class MakeIndexIterable:public Parent
	{
	public:
		class IteratorEnd
		{
		private:
			using counter_t = decltype(declval<Derived>().size()); /// cannot define in MakeIndexIterable, because of Derived
			const counter_t m_size;

		public:
			IteratorEnd(counter_t size) noexcept
				:m_size(size)
			{
			}
		};
		class Iterator:public MakeIterator<Iterator, ValueType>
		{
		private:
			using counter_t = decltype(declval<Derived>().size()); /// cannot define in MakeIndexIterable, because of Derived
			Derived* m_iter;
			counter_t m_index;

		public:
			Iterator(MakeIndexIterable* iter, counter_t index) noexcept
				:m_iter(static_cast<Derived*>(iter)), m_index(index)
			{
			}

			bool operator ==(const IteratorEnd& other) const noexcept
			{
				return m_index == other.m_size;
			}
			bool operator !=(const IteratorEnd& other) const noexcept
			{
				return m_index != other.m_size;
			}
			ValueType value() const noexcept
			{
				return (*m_iter)[m_index];
			}
			void next() noexcept
			{
				m_index++;
			}
			void previous() noexcept
			{
				m_index++;
			}
		};
		Iterator begin() noexcept
		{
			return Iterator(this, 0);
		}
		const IteratorEnd end() noexcept
		{
			return IteratorEnd(static_cast<Derived*>(this)->size());
		}
	};

	template <typename Derived, typename Parent = Empty>
	class MakeIterable:public Parent
	{
	public:
		class Iterator;

		Iterator begin() noexcept
		{
			return Iterator(static_cast<Derived*>(this));
		}
		IteratorEnd end() const noexcept
		{
			return IteratorEnd();
		}
	};

	template <typename Derived, typename Parent>
	class MakeIterable<Derived, Parent>::Iterator:public Derived::Iterator
	{
	public:
		INHERIT_ITERATOR(typename Derived::Iterator);
	};

	template <typename Derived, typename ValueType, typename Parent = Empty>
	class MakeIterableIterator :public MakeIterator<Derived, ValueType, Parent>
	{
	public:
		INHERIT_ITERATOR(MakeIterator<Derived, ValueType, Parent>);

		Derived& begin() noexcept
		{
			return *static_cast<Derived*>(this);
		}
		IteratorEnd end() const noexcept
		{
			return IteratorEnd();
		}
	};

	template <typename Iterable>
	class ReverseIterableWith
	{
	public:
		Iterable* const m_iterable;

		ReverseIterableWith(Iterable* iterable) :m_iterable(iterable)
		{
		}
		typename Iterable::Iterator::Reverse begin() const
		{
			return m_iterable->rbegin();
		}
		typename Iterable::Iterator::Reverse end() const
		{
			return m_iterable->rend();
		}
	};

	template <class Derived, class Parent = Empty> class MakeReverseIterable : public Parent
	{
	public:
		using ReverseIterable = ReverseIterableWith<Derived>;
		using ConstReverseIterable = ReverseIterableWith<const Derived>;

		ReverseIterable reverse() noexcept
		{
			return ReverseIterable(static_cast<Derived*>(this));
		}
		ConstReverseIterable reverse() const noexcept
		{
			return ConstReverseIterable(static_cast<const Derived*>(this));
		}
	};

	template <typename Iterable, typename LAMBDA>
	class FilterIterable :public MakeIterableIterator<FilterIterable<Iterable, LAMBDA>, decltype(declval<Iterable>().begin())>
	{
		INHERIT_ITERATOR(MakeIterableIterator<FilterIterable<Iterable, LAMBDA>, decltype(declval<Iterable>().begin())>);
	private:
		decltype(declval<Iterable>().begin()) m_iterator;
		decltype(declval<Iterable>().end()) m_iterator_end;
		LAMBDA m_condition;

	public:

		FilterIterable(Iterable& iter, LAMBDA&& lambda) noexcept
			:m_iterator(iter.begin()),
			m_iterator_end(iter.end()),
			m_condition(move(lambda))
		{
		}
		FilterIterable(Iterable& iter, const LAMBDA& lambda) noexcept
			:m_iterator(iter.begin()),
			m_iterator_end(iter.end()),
			m_condition(lambda)
		{
		}

		ValueType value() noexcept
		{
			return *m_iterator;
		}

		void next() noexcept
		{
			m_iterator++;
			while (m_iterator != m_iterator_end && !m_condition(m_iterator))
			{
				m_iterator++;
			}
		}

		bool isEnd() const noexcept
		{
			return m_iterator = m_iterator_end;
		}
	};

	template <typename Iterable, typename LAMBDA>
	FilterIterable<Iterable, LAMBDA> filterIterable(Iterable& iterable, LAMBDA&& lambda) noexcept
	{
		return { iterable, lambda };
	}

}
