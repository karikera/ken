#pragma once

#include <string>
#include <unordered_set>

#include <KR3/main.h>


namespace kr
{
	namespace _pri_
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
				friend SetWrapper<TK, TV>;
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

				ConstIterator& operator ++() noexcept
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

				bool operator ==(const ConstIterator& other) const noexcept
				{
					return m_raw == other.m_raw;
				}
				bool operator !=(const ConstIterator& other) const noexcept
				{
					return m_raw != other.m_raw;
				}
			};

			class Iterator
			{
				friend SetWrapper<TK, TV>;
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

				Iterator& operator ++() noexcept
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

				bool operator ==(const Iterator& other) const noexcept
				{
					return m_raw == other.m_raw;
				}
				bool operator !=(const Iterator& other) const noexcept
				{
					return m_raw != other.m_raw;
				}
			};

			SetWrapper() noexcept
			{
			}

			SetWrapper(initializer_list<TV> values) noexcept
			{
				for (const TV& v : values)
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

			std::pair<iterator, bool> insert(const TV& value) noexcept
			{
				return super::insert(value.template cast<ComponentTK>());
			}
			size_type erase(const TV& key) noexcept
			{
				return super::erase(key.template cast<ComponentTK>());
			}
			Iterator erase(Iterator iter) noexcept
			{
				return super::erase(iter.m_raw);
			}
			Iterator find(const TV& key) noexcept
			{
				return super::find(key.template cast<ComponentTK>());
			}
			ConstIterator find(const TV& key) const noexcept
			{
				return super::find(key.template cast<ComponentTK>());
			}
		};
	}

	template <typename T, bool referencedInput = false>
	class Set:public meta::if_t<IsMemory<T>::value, _pri_::SetWrapper<meta::if_t<referencedInput, Buffer, ABuffer>, T>, std::unordered_set<T> >
	{
		using Super = meta::if_t<IsMemory<T>::value, _pri_::SetWrapper<meta::if_t<referencedInput, Buffer, ABuffer>, T>, std::unordered_set<T> >;
	public:
		using Super::Super;
		using Super::find;
		using Super::end;

		bool has(const T& key) const noexcept
		{
			return find(key) != end();
		}
	};
}
