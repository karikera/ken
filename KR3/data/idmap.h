#pragma once

#include "bin_search.h"

namespace kr
{
	namespace map
	{
		template <typename Parent, typename Cmp = Comparator<Key<typename Parent::Component>>>
		class IdMap: public Parent
		{
		public:
			using Component = typename Parent::Component;
			using K = GetKeyType<Cmp>;
			using Search = Searcher<Component, Cmp>;

			IdMap() noexcept;
			IdMap(size_t capacity) noexcept;
			~IdMap() noexcept;

			bool insert(const Component &data) noexcept;
			bool replace(const Component &data) noexcept;
			bool remove(K key) noexcept;
			Component removeGet(K key) noexcept;
			bool removeGet(K key, Component* dest) noexcept;
			void removeByIndex(size_t idx) noexcept;
			Component removeByIndexGet(size_t idx) noexcept;
			Component get(K key) noexcept;

			K getEmptyKey(K from, K to) noexcept;
			K getEmptyKey(K from) noexcept;

			Component& getDataByIndex(size_t index) noexcept;
			const Component& getDataByIndex(size_t index) const noexcept;
			size_t getIndexByData(Component* ptr) noexcept;
			void forceSetByIndex(size_t index, Component data) noexcept;
			void forceInsertByIndex(size_t index, Component data) noexcept;
			void makeEmptyField(size_t size) noexcept;
			using Parent::begin;
			using Parent::end;
			using Parent::size;
			using Parent::removeMatchAllL;
			using Parent::removeMatchAll;

			template <typename LAMBDA> Component insertAlways(Component data, LAMBDA &&old = [](Component) {}) noexcept;
			template <typename LAMBDA> auto findAct(K key, LAMBDA &&lambda)->decltype(lambda(nullptr));
			template <typename LAMBDA, typename LAMBDA2> auto findAct(K key, LAMBDA &&found, LAMBDA2 &&notfound)->decltype(found(nullptr));

		protected:
			using Parent::resize;
			using Parent::push;
			using Parent::operator [];

		private:
			Component* _searchKeyLeft(K key) noexcept;
		};

		template <typename Parent, typename Cmp = Comparator<Key<typename Parent::Component>>>
		class SortedArray : public Parent
		{
		public:
			using Component = typename Parent::Component;
			using K = GetKeyType<Cmp>;
			using Search = Searcher<Component, Cmp>;

			SortedArray() noexcept;
			SortedArray(size_t capacity) noexcept;
			~SortedArray() noexcept;

			void insert(const Component &data) throws(NotEnoughSpaceException);
			void insert(Component&& data) throws(NotEnoughSpaceException);
			void rankingInsert(const Component & data, size_t limit) throws(NotEnoughSpaceException);

			void remove(K key) noexcept;
			Component removeGet(K key) noexcept;
			bool removeGet(K key, Component* dest) noexcept;
			void removeByIndex(size_t idx) noexcept;
			Component removeByIndexGet(size_t idx) noexcept;
			Component get(K key) noexcept;

			using Parent::begin;
			using Parent::end;
			using Parent::size;
			using Parent::empty;
			using Parent::resize;
			using Parent::operator [];
			void insertByIndex(size_t index, const Component &data) noexcept;
			void makeEmptyField(size_t size) noexcept;
			using Parent::removeMatchAllL;
			using Parent::removeMatchAll;

			template <typename LAMBDA> void rankingLoop(LAMBDA &&lambda) noexcept;
			template <typename LAMBDA> void rankingLoop_u(LAMBDA &&lambda) noexcept;
			template <typename LAMBDA> Component insertAlways(Component data, LAMBDA &&old = [](Component) {}) noexcept;
			template <typename LAMBDA> auto findAct(K key, LAMBDA &&lambda)->decltype(lambda(nullptr));
			template <typename LAMBDA, typename LAMBDA2> auto findAct(K key, LAMBDA &&found, LAMBDA2 &&notfound)->decltype(found(nullptr));

		protected:
			using Parent::push;

		private:
			Component* _searchKeyLeft(K key) noexcept;
		};
	}

	template <typename T, typename Cmp = Comparator<Key<T>>>
	using IdMap = map::IdMap<Array<T>, Cmp>;
	template <typename T, size_t SZ, typename Cmp = Comparator<Key<T>>>
	using BIdMap = map::IdMap<BArray<T, SZ>, Cmp>;
	template <typename T, typename Cmp = Comparator<Key<T>>>
	using SortedArray = map::SortedArray<Array<T>, Cmp>;
	template <typename T, size_t SZ, typename Cmp = Comparator<Key<T>>>
	using BSortedArray = map::SortedArray<BArray<T, SZ>, Cmp>;
}

#include "idmap.inl"
