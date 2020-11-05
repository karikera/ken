#pragma once

template <typename Parent, typename Cmp> 
kr::map::IdMap<Parent, Cmp>::IdMap() noexcept
{
}
template <typename Parent, typename Cmp> 
kr::map::IdMap<Parent, Cmp>::IdMap(size_t capacity) noexcept
	: Parent((size_t)0, capacity)
{
}
template <typename Parent, typename Cmp> 
kr::map::IdMap<Parent, Cmp>::~IdMap() noexcept
{
}
template <typename Parent, typename Cmp> 
bool kr::map::IdMap<Parent, Cmp>::insert(const Component &data) noexcept
{
	Component * beg = Parent::begin();
	return findAct(Cmp::getKey(data),
		[&](Component* p) { return false; },
		[&](Component* p) { Parent::insert(p - beg, data); return true; }
	);
}
template <typename Parent, typename Cmp> 
bool kr::map::IdMap<Parent, Cmp>::replace(const Component &data) noexcept
{
	return findAct(Cmp::getKey(data),
		[&](Component* p) { *p = data; return true; },
		[&](Component* p) { return false; }
	);
}
template <typename Parent, typename Cmp> 
bool kr::map::IdMap<Parent, Cmp>::remove(K key) noexcept
{
	return findAct(key,
		[this](Component* p) { 
			Parent::remove(p-begin());
			return true;
		},
		[](Component* p) {
			return false;
		}
	);
}
template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::Component kr::map::IdMap<Parent, Cmp>::removeGet(K key) noexcept
{
	return findAct(key,
		[this](Component* p) { return Parent::removeGet(p - begin()); },
		[](Component* p) { return (Component)nullptr; }
	);
}
template <typename Parent, typename Cmp> 
bool kr::map::IdMap<Parent, Cmp>::removeGet(K key, Component* dest) noexcept
{
	return findAct(key,
		[&](Component* p) { *dest = Parent::removeGet(p-begin()); return true; },
		[](Component* p) { return false; }
	);
}
template <typename Parent, typename Cmp> 
void kr::map::IdMap<Parent, Cmp>::removeByIndex(size_t idx) noexcept
{
	return Parent::remove(idx);
}
template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::Component kr::map::IdMap<Parent, Cmp>::removeByIndexGet(size_t idx) noexcept
{
	return Parent::removeGet(idx);
}
template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::Component kr::map::IdMap<Parent, Cmp>::get(K key) noexcept
{
	return findAct(key, [](Component* data) { return *data; }, [](Component*) { return (Component)0; });
}
template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::K kr::map::IdMap<Parent, Cmp>::getEmptyKey(K from, K to) noexcept
{
	Component* i = _searchKeyLeft(from);
	Component* end = Parent::end();
	while (i != end)
	{
		if (Cmp::getKey(*i) != from) break;
		i++;
		from++;
		if (from >= to) return (K)-1;
	}
	return from;
}
template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::K kr::map::IdMap<Parent, Cmp>::getEmptyKey(K from) noexcept
{
	Component* i = _searchKeyLeft(from);
	Component* end = Parent::end();
	while (i != end)
	{
		if (Cmp::getKey(*i) != from) break;
		i++;
		from++;
	}
	return from;
}

template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::Component* kr::map::IdMap<Parent, Cmp>::_searchKeyLeft(K key) noexcept
{
	auto lambda =[](Component* v) { return v; };
	return findAct(key, lambda,lambda);
}

template <typename Parent, typename Cmp> 
template <typename LAMBDA> 
typename kr::map::IdMap<Parent, Cmp>::Component kr::map::IdMap<Parent, Cmp>::insertAlways(Component data, LAMBDA &&old) noexcept
{
	return findAct(Cmp::getKey(data),
		[&](Component* p) { Component old = move(p);  p->value = move(data); return old; },
		[&](Component* p) { _shiftRight(p); new(p) Component(move(data)); return (Component)nullptr; }
	);
}
template <typename Parent, typename Cmp> 
template <typename LAMBDA> 
auto kr::map::IdMap<Parent, Cmp>::findAct(K key, LAMBDA &&lambda)->decltype(lambda(nullptr))
{
	Component * beg = Parent::begin();
	size_t sz = Parent::size();
	return Search::search(beg, sz, key, lambda, lambda);
}
template <typename Parent, typename Cmp> 
template <typename LAMBDA, typename LAMBDA2>
auto kr::map::IdMap<Parent, Cmp>::findAct(K key, LAMBDA &&found, LAMBDA2 &&notfound)->decltype(found(nullptr))
{
	return Search::search(Parent::begin(), Parent::size(), key, found, notfound);
}
template <typename Parent, typename Cmp> 
typename kr::map::IdMap<Parent, Cmp>::Component& kr::map::IdMap<Parent, Cmp>::getDataByIndex(size_t index) noexcept
{
	return Parent::begin()[index];
}
template <typename Parent, typename Cmp> 
const typename kr::map::IdMap<Parent, Cmp>::Component& kr::map::IdMap<Parent, Cmp>::getDataByIndex(size_t index) const noexcept
{
	return Parent::begin()[index];
}
template <typename Parent, typename Cmp> 
size_t kr::map::IdMap<Parent, Cmp>::getIndexByData(Component* ptr) noexcept
{
	return ptr - Parent::begin();
}
template <typename Parent, typename Cmp> 
void kr::map::IdMap<Parent, Cmp>::forceSetByIndex(size_t index, Component data) noexcept
{
	(*this)[index] = move(data);
}
template <typename Parent, typename Cmp> 
void kr::map::IdMap<Parent, Cmp>::forceInsertByIndex(size_t index, Component data) noexcept
{
	Parent::insert(index, move(data));
}
template <typename Parent, typename Cmp> 
void kr::map::IdMap<Parent, Cmp>::makeEmptyField(size_t size) noexcept
{
	Parent::resize(size);
}

template <typename Parent, typename Cmp> 
kr::map::SortedArray<Parent, Cmp>::SortedArray() noexcept
{
}
template <typename Parent, typename Cmp> 
kr::map::SortedArray<Parent, Cmp>::SortedArray(size_t capacity) noexcept
{
	Parent::reserve(capacity);
}
template <typename Parent, typename Cmp> 
kr::map::SortedArray<Parent, Cmp>::~SortedArray() noexcept
{
}
template <typename Parent, typename Cmp> 
void kr::map::SortedArray<Parent, Cmp>::insert(const Component &data) throws(NotEnoughSpaceException)
{
	Component * beg = Parent::begin();
	findAct(Cmp::getKey(data), [&](Component* p) { Parent::insert(p - beg, data); });
}
template <typename Parent, typename Cmp> 
void kr::map::SortedArray<Parent, Cmp>::rankingInsert(const Component & data, size_t limit) throws(NotEnoughSpaceException)
{
	if (size() < limit)
	{
		insert(data);
		return;
	}

	Component & last = (*this)[limit - 1];
	K lastKey = Cmp::getKey(last);
	K curKey = Cmp::getKey(data);
	if (Cmp::compare(curKey,lastKey) > 0) return;
	if (curKey != lastKey)
	{
		Component & last2 = (*this)[limit - 2];
		if (Cmp::getKey(last2) != lastKey)
		{
			resize(limit - 1);
		}
	}
	insert(data);
}
template <typename Parent, typename Cmp> 
void kr::map::SortedArray<Parent, Cmp>::remove(K key) noexcept
{
	findAct(key,
		[this](Component* p) { Parent::remove(p - begin()); },
		[](Component* p) { }
	);
}
template <typename Parent, typename Cmp> 
typename kr::map::SortedArray<Parent, Cmp>::Component kr::map::SortedArray<Parent, Cmp>::removeGet(K key) noexcept
{
	return findAct(key,
		[this](Component* p) { return Parent::removeGet(p - begin()); },
		[](Component* p) { return (Component)nullptr; }
	);
}
template <typename Parent, typename Cmp> 
bool kr::map::SortedArray<Parent, Cmp>::removeGet(K key, Component* dest) noexcept
{
	return findAct(key,
		[&](Component* p) { *dest = Parent::removeGet(p - begin()); return true; },
		[](Component* p) { return false; }
	);
}
template <typename Parent, typename Cmp> 
void kr::map::SortedArray<Parent, Cmp>::removeByIndex(size_t idx) noexcept
{
	Parent::remove(idx);
}
template <typename Parent, typename Cmp> 
typename kr::map::SortedArray<Parent, Cmp>::Component kr::map::SortedArray<Parent, Cmp>::removeByIndexGet(size_t idx) noexcept
{
	return Parent::removeGet(idx);
}
template <typename Parent, typename Cmp> 
typename kr::map::SortedArray<Parent, Cmp>::Component kr::map::SortedArray<Parent, Cmp>::get(K key) noexcept
{
	return findAct(key, [](Component* data) { return *data; }, [](Component*) { return (Component)0; });
}
template <typename Parent, typename Cmp> 
typename kr::map::SortedArray<Parent, Cmp>::Component* kr::map::SortedArray<Parent, Cmp>::_searchKeyLeft(K key) noexcept
{
	auto lambda = [](Component* v) { return v; };
	return findAct(key, lambda, lambda);
}

template <typename Parent, typename Cmp> 
template <typename LAMBDA>
void kr::map::SortedArray<Parent, Cmp>::rankingLoop(LAMBDA &&lambda) noexcept
{
	if(empty()) return;

	auto iter = begin();
	auto iterend = end();
	size_t ranking = 1;
	K key;
	{
		Component& c = *iter;
		key = Cmp::getKey(c);
		lambda(ranking, c);
	}
	size_t sz = size();
	iter++;

	size_t i = 1;
	for(;iter != iterend; iter++)
	{
		Component& c = *iter;
		uint curKey = Cmp::getKey(c);
		if (curKey != key)
		{
			key = curKey;
			ranking = i;
		}
		lambda(ranking, c);
		i++;
	}
}
template <typename Parent, typename Cmp> 
template <typename LAMBDA> 
void kr::map::SortedArray<Parent, Cmp>::rankingLoop_u(LAMBDA &&lambda) noexcept
{
	if (empty()) return;

	auto iter = begin();
	auto iterend = end();
	size_t ranking = 1;
	K key;
	{
		Component& c = *iter;
		key = Cmp::getKey(c);
		lambda(ranking, c);
	}
	size_t sz = size();
	iter++;
	
	for (; iter != iterend; iter++)
	{
		Component& c = *iter;
		uint curKey = Cmp::getKey(c);
		if (curKey != key)
		{
			key = curKey;
			ranking++;
		}
		lambda(ranking, c);
	}
}
template <typename Parent, typename Cmp> 
template <typename LAMBDA>
typename kr::map::SortedArray<Parent, Cmp>::Component kr::map::SortedArray<Parent, Cmp>::insertAlways(Component data, LAMBDA &&old) noexcept
{
	return findAct(Cmp::getKey(data),
		[&](Component* p) { Component old = move(p);  p->value = move(data); return old; },
		[&](Component* p) { _shiftRight(p); new(p) Component(move(data)); return (Component)nullptr; }
	);
}
template <typename Parent, typename Cmp> 
template <typename LAMBDA>
auto kr::map::SortedArray<Parent, Cmp>::findAct(K key, LAMBDA &&lambda)->decltype(lambda(nullptr))
{
	Component * beg = Parent::begin();
	size_t sz = Parent::size();
	return Search::search(beg, sz, key, lambda, lambda);
}
template <typename Parent, typename Cmp> 
template <typename LAMBDA, typename LAMBDA2>
auto kr::map::SortedArray<Parent, Cmp>::findAct(K key, LAMBDA &&found, LAMBDA2 &&notfound)->decltype(found(nullptr))
{
	return Search::search(Parent::begin(), Parent::size(), key, found, notfound);
}
template <typename Parent, typename Cmp> 
void kr::map::SortedArray<Parent, Cmp>::insertByIndex(size_t index, const Component &data) noexcept
{
	(*this)[index] = data;
}
template <typename Parent, typename Cmp> 
void kr::map::SortedArray<Parent, Cmp>::makeEmptyField(size_t size) noexcept
{
	Parent::resize(size);
}
