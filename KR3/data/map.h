#pragma once

#include <string>
#include <unordered_map>

#include <KR3/main.h>

namespace kr
{
	template <typename TI, typename TK, typename TV>
	class MapWrapper:public std::unordered_map<TK, TV>
	{
	private:
		using ComponentTK = typename TK::Component;
	public:
		using key_type = TI;
		using mapped_type = TV;

		using super = std::unordered_map<TK, TV>;
		using typename super::iterator;
		using typename super::const_iterator;
		using typename super::size_type;
		using rawpair = std::pair<TK, TV>;
		using super::begin;
		using super::end;

		MapWrapper() noexcept
		{
		}
		MapWrapper(initializer_list<std::pair<TI, TV>> pairs) noexcept
		{
			for (auto & pair : pairs)
			{
				insert(pair.first, pair.second);
			}
		}

		std::pair<iterator, bool> insert(const TI &key, TV value)
		{
			return super::insert(rawpair(key.template cast<ComponentTK>(), move(value)));
		}
		TV& operator [](const TI &key)
		{
			return (*(super*)this)[key.template cast<ComponentTK>()];
		}
		const TV& operator [](const TI &key) const
		{
			return (*(super*)this)[key.template cast<ComponentTK>()];
		}
		size_type erase(const TI &key)
		{
			return super::erase(key.template cast<ComponentTK>());
		}
		iterator erase(iterator iter)
		{
			return super::erase(iter);
		}
		iterator find(const TI& key)
		{
			return super::find(key.template cast<ComponentTK>());
		}
		const_iterator find(const TI& key) const
		{
			return super::find(key.template cast<ComponentTK>());
		}
		bool has(const TI& key) const noexcept
		{
			return find(key) != end();
		}

		const TV & get(const TI &key, const TV & defValue) const noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return defValue;
			return iter->second;
		}
		template <typename LAMBDA>
		bool ifGet(const TI &key, LAMBDA & lambda) const noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return false;
			lambda(iter->second);
			return true;
		}

	};

	template <typename TI, typename TV> using Map = meta::if_t<IsBuffer<TI>::value, MapWrapper<TI, ABuffer, TV>, std::unordered_map<TI, TV>>;
	template <typename TI, typename TV> using ReferenceMap = meta::if_t<IsBuffer<TI>::value, MapWrapper<TI, Buffer, TV>, std::unordered_map<TI, TV>>;

}
