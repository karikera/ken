
#pragma once

#include <string>
#include <unordered_map>

#include <KR3/main.h>

namespace kr
{
	namespace _pri_
	{
		template <typename TI, typename TK, typename TV>
		class MapWrapper :public std::unordered_map<TK, TV>
		{
			using Super = std::unordered_map<TK, TV>;
		private:
			using ComponentTK = typename TK::Component;
		public:
			using key_type = TI;
			using mapped_type = TV;

			using typename Super::iterator;
			using typename Super::const_iterator;
			using typename Super::size_type;
			using rawpair = std::pair<TK, TV>;
			using Super::begin;
			using Super::end;

			MapWrapper() noexcept
			{
			}
			MapWrapper(initializer_list<std::pair<TI, TV>> pairs) noexcept
			{
				for (auto& pair : pairs)
				{
					insert(pair.first, pair.second);
				}
			}

			std::pair<iterator, bool> insert(const TI& key, TV value)
			{
				return Super::insert(rawpair(key.template cast<ComponentTK>(), move(value)));
			}
			TV& operator [](const TI& key)
			{
				return (*(Super*)this)[key.template cast<ComponentTK>()];
			}
			const TV& operator [](const TI& key) const
			{
				return (*(Super*)this)[key.template cast<ComponentTK>()];
			}
			size_type erase(const TI& key)
			{
				return Super::erase(key.template cast<ComponentTK>());
			}
			iterator erase(iterator iter)
			{
				return Super::erase(iter);
			}
			iterator find(const TI& key)
			{
				return Super::find(key.template cast<ComponentTK>());
			}
			const_iterator find(const TI& key) const
			{
				return Super::find(key.template cast<ComponentTK>());
			}

		};

	}

	template <typename TI, typename TV, bool referencedInput = false>
	class Map:public meta::if_t<IsMemBuffer<TI>::value, _pri_::MapWrapper<TI, meta::if_t<referencedInput, Buffer, ABuffer>, TV>, std::unordered_map<TI, TV> >
	{
		using Super = meta::if_t<IsMemBuffer<TI>::value, _pri_::MapWrapper<TI, meta::if_t<referencedInput, Buffer, ABuffer>, TV>, std::unordered_map<TI, TV> >;
	public:
		using typename Super::const_iterator;
		using Super::Super;
		using Super::find;
		using Super::end;

		bool has(const TI& key) const noexcept
		{
			return find(key) != end();
		}

		const TV& get(const TI& key, const TV& defValue) const noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return defValue;
			return iter->second;
		}

		template <typename LAMBDA>
		bool ifGet(const TI& key, LAMBDA& lambda) const noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return false;
			lambda(iter->second);
			return true;
		}
	};
}
