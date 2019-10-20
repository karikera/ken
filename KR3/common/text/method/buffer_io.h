#pragma once

namespace kr
{
	namespace ary
	{
		namespace method
		{
			template <class Parent> class BufferIOMethod
				:public Parent
			{
				CLASS_HEADER(BufferIOMethod, Parent);
			public:
				INHERIT_ARRAY();

				using Super::_resize;
				using Super::_extend;
				using Super::_setSize;
				using Super::_realloc;
				using Super::_alloc;
				using Super::_clear;
				using Super::_shiftRight;
				using Super::Super;
				using Super::data;
				using Super::begin;
				using Super::end;
				using Super::size;
				using Super::sizeBytes;
				using Super::capacity;
				using Super::empty;
				using Super::reserve;
				using Super::szable;
				using Super::fill;
				using Super::zero;
				using Super::operator =;

				void clear() noexcept
				{
					mema::dtor(begin(), size());
					_clear();
				}
				void truncate() noexcept
				{
					this->~BufferIOMethod();
					new(this) BufferIOMethod(nullptr);
				}
				void pick(Component * pos) noexcept
				{
					_assert(begin() <= pos && pos < end());
					size_t sz = size() - 1;
					_resize(sz);

					Component* last = begin() + sz;
					if (pos != last) *pos = move(*last);

					callDestructor(last);
				}
				void pick(size_t i) noexcept
				{
					_assert(i < size());
					size_t sz = size() - 1;
					_resize(sz);

					Component* data = begin();
					Component* last = data + sz;
					if (i != sz) data[i] = move(*last);

					callDestructor(last);
				}
				InternalComponent pickGet(size_t i) noexcept
				{
					_assert(i < size());
					size_t sz = size() - 1;
					_resize(sz);

					InternalComponent* data = (InternalComponent*)begin();
					InternalComponent* pos = data + i;
					InternalComponent* last = data + sz;

					if (pos == last)
						return move(*pos);

					InternalComponent out = move(*pos);
					*pos = move(*last);
					callDestructor(last);
					return move(out);
				}
				InternalComponent pickRandom() noexcept
				{
					return pickGet(g_random.getDword() % size());
				}
				void fill(const InternalComponent &chr, size_t sz) throws(NotEnoughSpaceException)
				{
					KR_DEFINE_MMEM();
					if (std::is_trivially_default_constructible<InternalComponent>::value)
					{
						_resize(sz);
						memm::set(begin(), chr, sz);
					}
					else
					{
						size_t osz = size();
						_resize(sz);
						if (sz < osz)
						{
							mema::subs_fill(begin(), chr, sz);
						}
						else
						{
							mema::subs_fill(begin(), chr, osz);
							mema::ctor_fill(begin() + osz, chr, sz - osz);
						}
					}
				}
				void zero(size_t sz) throws(NotEnoughSpaceException)
				{
					_resize(sz);
					memset(data(), 0, sizeBytes());
				}
				template <typename T> void copy(const T& _copy)
				{
					bufferize_t<T, Component> buffer = _copy;
					size_t sz = buffer.size();
					resize(sz, sz + buffer.szable);
					size_t cap = capacity();
					buffer.copyTo(begin());
				}
				void copy(const Component* arr, size_t sz)
				{
					KR_DEFINE_MMEM();
					if (std::is_trivially_default_constructible<Component>::value)
					{
						_resize(sz);
						memm::copy(begin(), arr, sz);
					}
					else
					{
						size_t osz = size();
						if (sz < osz)
						{
							_resize(sz);
							mema::subs_copy(begin(), arr, sz);
						}
						else
						{
							_resize(sz);
							mema::subs_copy(begin(), arr, osz);
							mema::ctor_copy(begin() + osz, arr + osz, sz - osz);
						}
					}
				}
				void equalOperator(Ref ref)
				{
					copy(ref.data(), ref.size());
				}
			
				void pop() throws(EofException)
				{
					size_t osize = size();
					if(osize == 0)
						throw EofException();
					_resize(osize-1);
				}
				InternalComponent popGet() throws(EofException)
				{
					size_t osize = size();
					if (osize == 0)
						throw EofException();
					osize--;
					InternalComponent out = move(((InternalComponent*)begin())[osize]);
					_resize(osize);
					return move(out);
				}

				void cut_self(const ComponentRef* newend) noexcept
				{
					_assert(begin() <= newend && newend <= end());
					_resize(newend - (ComponentRef*)begin());
				}
				void cut_self(Ref _v) noexcept
				{
					return cut_self(_v.begin());
				}
				void cut_self(size_t _len) noexcept
				{
					_len = mint(_len, size());
					_resize(_len);
				}

				InternalComponent* push(InternalComponent data) throws(NotEnoughSpaceException)
				{
					size_t osize = size();
					_resize(osize + 1);
					InternalComponent * comp = begin() + osize;
					new(comp) InternalComponent(move(data));
					return comp;
				}
				void insert(size_t i, InternalComponent value) throws(NotEnoughSpaceException)
				{
					_assert(i <= size());
					_shiftRight(i);
					new(begin() + i) InternalComponent(move(value));
				}
				void insert(size_t i, Ref arr) throws(NotEnoughSpaceException)
				{
					_assert(i <= size());
					size_t cnt = arr.size();
					mema::ctor_copy(prepareAt(i, cnt), arr.begin(), cnt);
				}
				Component* prepareAt(size_t i, size_t cnt) throws(NotEnoughSpaceException)
				{
					_assert(i <= size());
					_shiftRight(i, cnt);
					return begin() + i;
				}
				void remove_p(Component * axis) noexcept
				{
					size_t sz = size();
					_assert((size_t)(axis - begin()) < sz);
					sz--;
					kr::mema::ctor_move_d(axis, axis + 1, begin() + sz - axis);
					_setSize(sz);
				}
				void remove(size_t i) noexcept
				{
					size_t sz = size();
					_assert(i < sz);
					Component * axis = begin() + i;
					sz--;
					kr::mema::ctor_move_d(axis, axis + 1, sz - i);
					_setSize(sz);
				}
				void remove(size_t i, size_t cnt) noexcept
				{
					size_t sz = size();
					_assert(i+cnt <= sz);
					Component * axis = begin() + i;
					kr::mema::ctor_move_d(axis, axis + cnt, sz - i - cnt);
					_setSize(sz - cnt);
				}
				InternalComponent removeGet(size_t i) noexcept
				{
					size_t sz = size();
					_assert(i < sz);
					InternalComponent * axis = (InternalComponent*)begin() + i;
					InternalComponent out = move(*axis);
					kr::mema::ctor_move_d(axis, axis + 1, sz - i - 1);
					_setSize(sz - 1);
					return move(out);
				}
				void resize(size_t nsize) throws(NotEnoughSpaceException)
				{
					size_t sz = size();
					_resize(nsize);
					if (nsize > sz)
						mema::ctor((InternalComponent*)begin() + sz, nsize - sz);
				}
				void resizeUp(size_t nsize) throws(NotEnoughSpaceException)
				{
					size_t sz = size();
					if (nsize <= sz) return;
					_resize(nsize);
					mema::ctor((InternalComponent*)begin() + sz, nsize - sz);
				}
				template <typename ... ARGS>
				void initResize(size_t nsize, const ARGS & ... args) throws(NotEnoughSpaceException)
				{
					size_t sz = size();
					_resize(nsize);
					if (nsize > sz)
					{
						Component * beg = begin();
						Component * end = beg + nsize;
						beg += sz;
						do
						{
							new(beg) Component(args ...);
							beg++;
						}
						while (beg != end);
					}
				}
				void alloc(size_t nsize) throws(NotEnoughSpaceException)
				{
					clear();
					_resize(nsize);
					mema::ctor(begin(), nsize);
				}
				void resize(size_t nsize, size_t ncapacity) throws(NotEnoughSpaceException)
				{
					reserve(ncapacity);
					resize(nsize);
				}
				template <typename LAMBDA>
				bool removeMatchL(const LAMBDA &lambda) throws(...)
				{
					Component* i = begin();
					Component* to = end();

					for (;i != to; i++)
					{
						if (!lambda(*i)) continue;
						remove_p(i);
						return true;
					}
					return false;
				}
				bool removeMatch(const InternalComponent &value) noexcept
				{
					return removeMatchL([&](const InternalComponent& v)->bool { return v == value; });
				}
				void removeMatchAll(const InternalComponent &value) noexcept
				{
					removeMatchAllL([&](const InternalComponent& v)->bool{ return v == value; });
				}
				template <typename LAMBDA>
				void removeMatchAllL(const LAMBDA &lambda) throws(...)
				{
					Component* ptr = begin();
					Component* to = end();

					for (;;)
					{
						if (ptr == to) return;
						if (lambda(*ptr)) break;
						ptr++;
					}

					Component* moveto = ptr;

					callDestructor(ptr++);

					while (ptr != to)
					{
						if (lambda(*ptr))
						{
							callDestructor(ptr++);
						}
						else
						{
							new(moveto++) Component(move(*ptr));
							callDestructor(ptr++);
						}
					}

					_setSize(moveto - begin());
				}

				template <typename S> void serialize(S &s) throws(...)
				{
					s.serializeSize(this);
					for (Component & c : *this)
					{
						s & c;
					}
				}

				constexpr BufferIOMethod() = default;
				constexpr BufferIOMethod(const BufferIOMethod& _copy) = default;
				constexpr BufferIOMethod(BufferIOMethod && _mv) = default;

				BufferIOMethod(const Ref & data) throws(NotEnoughSpaceException)
				{
					size_t sz = data.size();
					_alloc(sz, sz + szable);
					Component * beg = begin();
					mema::ctor(beg, sz);
					resize(data.copyTo(beg));
				}

				template <typename _Derived, class _Parent>
				BufferIOMethod(const Printable<_Derived, Component, _Parent> & _data) throws(NotEnoughSpaceException)
				{
					_data.writeTo(this);
				}
				template <typename _Derived, bool szable, bool b, class _Parent>
				BufferIOMethod(const Bufferable<_Derived, BufferInfo<Component, false, szable, b, _Parent>> & _data) throws(NotEnoughSpaceException)
				{
					size_t sz = _data.size();
					_alloc(sz, sz + szable);
					Component * beg = begin();
					mema::ctor(beg, sz);
					resize(_data.copyTo(beg));
				}
				template <typename _Derived, bool szable, bool b, class _Parent>
				BufferIOMethod(const Bufferable<_Derived, BufferInfo<AutoComponent, false, szable, b, _Parent>> & _data) throws(NotEnoughSpaceException)
				{
					size_t sz = _data.template sizeAs<Component>();
					_alloc(sz, sz + szable);
					Component * beg = begin();
					mema::ctor(beg, sz);
					resize(_data.template copyTo<Component>(beg));
				}
				template <typename _Derived, bool a, bool b, class _Parent>
				explicit BufferIOMethod(Bufferable<_Derived, BufferInfo<Component, true, a, b, _Parent>> && _mv) throws(NotEnoughSpaceException)
				{
					_move(_mv.begin(), _mv.size());
				}

				Component * c_str() noexcept
				{
					*this << nullterm;
					return begin();
				}

				template <typename ... ARGS>
				static Self concat(const ARGS & ... args) throws(NotEnoughSpaceException)
				{
					Self text;
					text.prints(args ...);
					return text;
				}
			protected:
				friend typename Parent::Data;
				template <typename, class>
				friend class data::WritableForm;
			};
		}
	}
}