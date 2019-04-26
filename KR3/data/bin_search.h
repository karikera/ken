#pragma once

#include "../main.h"

namespace kr
{
	template <typename T> struct Key;


	namespace _pri_
	{
		template <typename T, bool iscls> struct _Key;
		template <typename T> struct _Key<T, true>
		{
			static auto getKey(const T &t) noexcept -> decltype(t.getKey())
			{
				return t.getKey();
			}
		};
		template <typename T> struct _Key<Keep<T>, true>
		{
			static auto getKey(const T *t) noexcept -> decltype(t->getKey())
			{
				return t->getKey();
			}
		};
		template <typename T> struct _Key<Must<T>, true>
		{
			static auto getKey(const T *t) noexcept -> decltype(t->getKey())
			{
				return t->getKey();
			}
		};
		template <typename T> struct _Key<T*, false>
		{
			static auto getKey(const T *t) noexcept -> decltype(t->getKey())
			{
				return t->getKey();
			}
		};
		template <typename T> struct _Key<T, false>
		{
			static T getKey(const T &t) noexcept
			{
				return t;
			}
		};

		template <typename T, bool iscls> struct _Comparator;
		template <typename T> struct _Comparator<T, true>
		{
			static intptr_t compare(const T& a, const T&b)
			{
				return a.compare(b);
			}
		};
		template <typename T> struct _Comparator<T*, false>
		{
			static intptr_t compare(const T * a, const T * b)
			{
				return a->compare(b);
			}
		};
		template <typename T> struct _Comparator<T, false>
		{
			static intptr_t compare(const T& a, const T&b) noexcept
			{
				using K = Key<T>;
				return (intptr_t)K::getKey(a) - (intptr_t)K::getKey(b);
			}
		};

	}

	template <typename T> struct Key: _pri_::_Key<T, std::is_class<T>::value>
	{
	};
	template <typename K>
	using GetKeyType = decltype(K::getKey(zerovar));

	template <typename Key> struct Comparator : _pri_::_Comparator<GetKeyType<Key>, std::is_class<GetKeyType<Key>>::value>, Key
	{
	};
	struct GetPointerAsKey
	{
		static uintptr_t getKey(const void * value) noexcept
		{
			return (uintptr_t)value;
		}
	};

	template <typename T, typename Cmp = Comparator<Key<T>>>
	class Searcher:public Cmp
	{
	public:
		using Cmp::compare;
		using Cmp::getKey;

		using K = GetKeyType<Cmp>;
		static T* searchPointer(T* L, size_t size, K key)
		{
			L--;
			T* R = L + size;
			T* E = R;

			while (R > L + 1)
			{
				T* C = (R - L) / 2 + L;
				if (compare(getKey(*C), key) < 0) L = C;
				else R = C;
			}

			if (R == E) return nullptr;
			else if (compare(getKey(*R), key) == 0) return R;
			else return nullptr;
		}
		template <typename FOUND, typename NOTFOUND>
		static auto search(T* L, size_t size, K key, FOUND found, NOTFOUND notfound)->decltype(found((T*)0))
		{
			T* R = L + size;
			T* E = R;
			L--;

			while (R > L + 1)
			{
				T* C = (R - L) / 2 + L;
				if (compare(getKey(*C), key) < 0) L = C;
				else R = C;
			}

			if (R == E) return notfound(E);
			else if (compare(getKey(*R), key) == 0) return found(R);
			else return notfound(R);
		}
		static const T* searchLeft(const T* L, size_t size, K key)
		{
			L--;
			const T* R = L + size;
			const T* E = R;

			while (R > L + 1)
			{
				const T* C = (R - L) / 2 + L;
				if (compare(getKey(*C), key) < 0) L = C;
				else R = C;
			}

			return R;
		}

		static void swap(T & a, T& b)
		{
			T tmp = move(a);
			a = move(b);
			b = move(tmp);
		}
		static void sort(T* begin, T* end)
		{
			switch (end - begin)
			{
			case 0: return;
			case 1: return;
			case 2:
				if (compare(getKey(begin[0]), getKey(begin[1])) > 0)
				{
					swap(begin[0], begin[1]);
				}
				return;
			}
			T * endm = end - 1;
			T * L = begin;
			T * R = endm;
			T * C = endm;
			K Ckey = getKey(*C);

			while (L != R)
			{
				for (;;)
				{
					if (compare(getKey(*L), Ckey) > 0) break;
					L++;
					if (L == R) goto _fin;
				}
				for (;;)
				{
					R--;
					if (L == R) goto _fin;
					if (compare(getKey(*R), Ckey) < 0) break;
				}
				swap(*L++, *R);
			}
		_fin:
			if (L != C) swap(*L, *C);
			if (L > begin + 1) sort(begin, L);
			L++;
			if (L < endm) sort(L, end);
		}
		static void sort(WView<T> array)
		{
			return sort(array.begin(), array.end());
		}
		static bool isSorted(View<T> array)
		{
			return isSorted(array.begin(), array.end());
		}
		static bool isSorted(const T * begin, const T * end) noexcept
		{
			if (end - begin < 2) return true;
			K prevKey = getKey(begin);
			begin++;

			for (; begin != end; begin++)
			{
				K nkey = getKey(*begin);
				if (compare(prevKey, nkey) > 0) return false;
				prevKey = nkey;
			}
			return true;
		}
	};
}
