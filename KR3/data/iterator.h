#pragma once

#include "../main.h"

#define INHERIT_ITERATOR(...) \
	using Super = __VA_ARGS__;\
	using Super::operator !=;\
	using Super::operator ==;\
	using Super::operator ->;\
	using Super::operator *;\
	using Super::operator --;\
	using Super::operator ++;\
	using Super::Super;

namespace kr
{

	template <class ITERATOR, typename C> class TIterator
	{
	public:
		using Component = C;

		TIterator(nullptr_t) noexcept
			:m_pt(nullptr)
		{
		}
		TIterator(C * pt) noexcept
			:m_pt(pt)
		{
		}

		template <typename I2> bool operator ==(const TIterator<I2, C> &o) const
		{
			return m_pt == o.m_pt;
		}
		template <typename I2> bool operator !=(const TIterator<I2, C> &o) const
		{
			return m_pt != o.m_pt;
		}
		C* operator ->() const
		{
			return m_pt;
		}
		C& operator *() const
		{
			return *m_pt;
		}
		operator C*() const
		{
			return m_pt;
		}

		C* operator --(int)
		{
			C* pt = m_pt;
			--(*static_cast<ITERATOR*>(this));
			return pt;
		}
		C* operator ++(int)
		{
			C* pt = m_pt;
			++(*static_cast<ITERATOR*>(this));
			return pt;
		}

		class Reverse:public TIterator<Reverse,C>
		{
		public:
			INHERIT_ITERATOR(TIterator<Reverse, C>);
		
			Reverse& operator ++()
			{
				return (Reverse&)--(ITERATOR&)*this;
			}
			Reverse& operator --()
			{
				return (Reverse&)++(ITERATOR&)*this;
			}
		};

	protected:
		C * m_pt;
	};

	template <class ITERABLE, class PARENT = Empty> class TIterable: public PARENT
	{
	public:

		class ReverseIterable
		{
		public:
			ReverseIterable(ITERABLE & iterable):m_iterable(iterable)
			{
			}
			typename ITERABLE::Iterator::Reverse begin()
			{
				return m_iterable.rbegin();
			}
			typename ITERABLE::Iterator::Reverse end()
			{
				return m_iterable.rend();
			}

		private:
			ITERABLE & m_iterable;
		};

		ReverseIterable reverse()
		{
			return ReverseIterable(*static_cast<ITERABLE*>(this));
		}
	};

	template <class Iterable>
	class TIndexIterable
	{
	public:
		class Iterator
		{
		private:
			using counter_t = decltype(((Iterable*)nullptr)->size());
			Iterable * m_iter;
			counter_t m_index;

		public:
			Iterator(TIndexIterable * iter, counter_t index) noexcept
				:m_iter(static_cast<Iterable*>(iter)), m_index(index)
			{
			}

			bool operator ==(const Iterator & other) noexcept
			{
				_assert(m_iter == other.m_iter);
				return m_index == other.m_index;
			}
			bool operator !=(const Iterator & other) noexcept
			{
				_assert(m_iter == other.m_iter);
				return m_index != other.m_index;
			}
			auto operator *() noexcept->decltype((*m_iter)[m_index])
			{
				return (*m_iter)[m_index];
			}
			auto operator ->() noexcept->decltype((*m_iter)[m_index])
			{
				return &(*m_iter)[m_index];
			}
			Iterator & operator ++() noexcept
			{
				m_index++;
				return *this;
			}
			Iterator & operator --() noexcept
			{
				m_index--;
				return *this;
			}
			const Iterator operator ++(int) noexcept
			{
				Iterator old = *this;
				m_index++;
				return old;
			}
			const Iterator operator --(int) noexcept
			{
				Iterator old = *this;
				m_index--;
				return old;
			}
		};
		Iterator begin() noexcept
		{
			return Iterator(this, 0);
		}
		Iterator end() noexcept
		{
			return Iterator(this, static_cast<Iterable*>(this)->size());
		}
		
	};

}
