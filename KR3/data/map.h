
#pragma once

#include <string>
#include <unordered_map>

#include <KR3/main.h>

namespace kr
{
	namespace _pri_
	{
		class MapKeyData;
		class MapKey;
		class MapKeyStatic;
	}
}

template <>
struct std::hash<kr::_pri_::MapKeyData>
{
	size_t operator ()(const kr::_pri_::MapKeyData& key) const noexcept;
};

template <>
struct std::hash<kr::_pri_::MapKey>: std::hash<kr::_pri_::MapKeyData>
{
};

template <>
struct std::hash<kr::_pri_::MapKeyStatic>: std::hash<kr::_pri_::MapKeyData>
{
};

namespace kr
{
	namespace _pri_
	{
		class MapKeyData
		{
			friend std::hash<kr::_pri_::MapKeyData>;
		protected:
			const void* m_buffer;
			size_t m_size;

		public:
			template <typename T>
			operator View<T>() const noexcept
			{
				_assert(m_size % sizeof(T) == 0);
				return View<T>((T*)m_buffer, (T*)((byte*)m_buffer + m_size));
			}
			bool operator ==(const MapKeyData& other) const noexcept;
			bool operator !=(const MapKeyData& other) const noexcept;
		};

		class MapKey:public MapKeyData
		{
		public:
			template <typename Derived, typename Info>
			MapKey(const buffer::Memory<Derived, Info>& buffer) noexcept
			{
				using C = typename Info::Component;
				m_size = buffer.size() * sizeof(C);
				m_buffer = krmalloc(m_size);
				buffer.copyTo((C*)m_buffer);
			}
			~MapKey() noexcept;

			MapKey(const MapKey&) = delete;
			MapKey(MapKey&& _move) noexcept;
		};

		class MapKeyStatic :public MapKeyData
		{
		public:
			template <typename Derived, typename C, bool _szable, bool _readonly, typename _Parent>
			MapKeyStatic(const buffer::Memory<Derived, BufferInfo<C, method::Memory, _szable, _readonly, _Parent> >& buffer) noexcept
			{
				m_buffer = buffer.data();
				m_size = buffer.bytes();
			}
			operator const MapKey& () const noexcept;
		};


		template <typename TI, typename TV, bool referencedInput>
		class MapWrapper :public std::unordered_map<meta::if_t<referencedInput, MapKeyStatic, MapKey>, TV>
		{
			using Super = std::unordered_map<meta::if_t<referencedInput, MapKeyStatic, MapKey>, TV>;
		public:
			using key_type = TI;
			using mapped_type = TV;
			using inner_type = meta::if_t<referencedInput, MapKeyStatic, MapKey>;

			using typename Super::iterator;
			using typename Super::const_iterator;
			using typename Super::size_type;
			using rawpair = std::pair<inner_type, TV>;
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
				return Super::insert(rawpair(key, move(value)));
			}
			TV& operator [](const TI& key)
			{
				return (*(Super*)this)[key];
			}
			const TV& operator [](const TI& key) const
			{
				return (*(Super*)this)[key];
			}
			size_type erase(const TI& key)
			{
				MapKeyStatic data = key;
				return Super::erase(data);
			}
			iterator erase(iterator iter)
			{
				return Super::erase(iter);
			}
			iterator find(const TI& key)
			{
				MapKeyStatic data = key;
				return Super::find(data);
			}
			const_iterator find(const TI& key) const
			{
				MapKeyStatic data = key;
				return Super::find(data);
			}

		};

	}

	template <typename TI, typename TV, bool referencedInput>
	class Map:public meta::if_t<IsMemory<TI>::value, _pri_::MapWrapper<TI, TV, referencedInput>, std::unordered_map<TI, TV> >
	{
		using Super = meta::if_t<IsMemory<TI>::value, _pri_::MapWrapper<TI, TV, referencedInput>, std::unordered_map<TI, TV> >;
	public:
		using typename Super::const_iterator;
		using Super::Super;
		using Super::find;
		using Super::begin;
		using Super::end;

		bool has(const TI& key) const noexcept
		{
			return find(key) != end();
		}

		TV& get(const TI& key, const TV& defValue) noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return defValue;
			return iter->second;
		}
		const TV& get(const TI& key, const TV& defValue) const noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return defValue;
			return iter->second;
		}

		template <typename LAMBDA>
		bool ifGet(const TI& key, LAMBDA&& lambda) const noexcept
		{
			const_iterator iter = find(key);
			if (iter == end()) return false;
			lambda(iter->second);
			return true;
		}
	};
}
