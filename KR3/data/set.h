#pragma once

#include <string>
#include <unordered_set>

#include <KR3/main.h>


namespace kr
{
	template <typename TK, typename TV>
	class SetWrapper :public std::unordered_set<TK>
	{
	private:
		using ComponentTV = typename TV::Component;
		using ComponentTK = typename TK::Component;
	public:
		using super = std::unordered_set<TK>;
		using typename super::iterator;
		using typename super::const_iterator;
		using typename super::size_type;
		using super::begin;
		using super::end;
		using super::size;

		class ConstIterator
		{
		private:
			const_iterator m_raw;

		public:
			ConstIterator(const_iterator raw) noexcept
				:m_raw(raw)
			{
			}

			TV operator *() noexcept
			{
				return (TV)m_raw->first;
			}

			ConstIterator &operator ++() noexcept
			{
				++m_raw;
				return *this;
			}
			ConstIterator operator ++(int) noexcept
			{
				iterator ori = m_raw;
				++m_raw;
				return ori;
			}

			bool operator ==(const ConstIterator & other) const noexcept
			{
				return m_raw == other.m_raw;
			}
			bool operator !=(const ConstIterator & other) const noexcept
			{
				return m_raw != other.m_raw;
			}
		};

		class Iterator
		{
		private:
			iterator m_raw;

		public:
			Iterator(iterator raw) noexcept
				:m_raw(raw)
			{
			}

			const TV operator *() noexcept
			{
				return m_raw->template cast<ComponentTV>();
			}

			Iterator &operator ++() noexcept
			{
				++m_raw;
				return *this;
			}
			Iterator operator ++(int) noexcept
			{
				iterator ori = m_raw;
				++m_raw;
				return ori;
			}

			bool operator ==(const Iterator & other) const noexcept
			{
				return m_raw == other.m_raw;
			}
			bool operator !=(const Iterator & other) const noexcept
			{
				return m_raw != other.m_raw;
			}
		};

		SetWrapper() noexcept
		{
		}

		SetWrapper(initializer_list<TV> values) noexcept
		{
			for (const TV &v : values)
			{
				insert(v);
			}
		}

		Iterator begin() noexcept
		{
			return super::begin();
		}
		Iterator end() noexcept
		{
			return super::end();
		}

		ConstIterator begin() const noexcept
		{
			return super::begin();
		}
		ConstIterator end() const noexcept
		{
			return super::end();
		}

		std::pair<iterator, bool> insert(const TV & value) noexcept
		{
			return super::insert(value.template cast<ComponentTK>());
		}
		size_type erase(const TV &key) noexcept
		{
			return super::erase(key.template cast<ComponentTK>());
		}
		Iterator erase(Iterator iter) noexcept
		{
			return super::erase(iter);
		}
		Iterator find(const TV& key) noexcept
		{
			return super::find(key.template cast<ComponentTK>());
		}
		ConstIterator find(const TV& key) const noexcept
		{
			return super::find(key.template cast<ComponentTK>());
		}
		bool has(const TV& key) const noexcept
		{
			return find(key) != end();
		}
	};

	template <typename T> using Set = meta::if_t<IsBuffer<T>::value, SetWrapper<ABuffer, T>, std::unordered_set<T>>;
	template <typename T> using ReferenceSet = meta::if_t<IsBuffer<T>::value, SetWrapper<Buffer, T>, std::unordered_set<T>>;

}
