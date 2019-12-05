#pragma once

#include "../container.h"

namespace kr
{
	namespace ary
	{
		template <typename C, 
			bool _movable,
			bool _readable,
			bool _extendable,
			bool _writable,
			class Parent>
		class Common: public Container<C, !_writable, Parent>
		{
			CLASS_HEADER(Common, Container<C, !_writable, Parent>);
		public:
			INHERIT_COMPONENT();
			static constexpr bool movable = _movable;
			static constexpr bool readable = _readable;
			static constexpr bool extendable = _extendable;
			static constexpr bool writable = _writable;
		};
		template <class Parent, class Data> 
		class WritableForm :public Parent
		{
			CLASS_HEADER(WritableForm, Parent);
		public:
			INHERIT_COMPONENT();

			using Parent::Parent;
			using Parent::$begin;
			using Parent::$size;
			using Parent::$capacity;

			using Parent::_setSize;
			using Parent::_realloc;
			using Parent::_alloc;

			ComponentRef* $_extend(size_t inc) throws(NotEnoughSpaceException)
			{
				size_t sz = $size();
				_resize(sz + inc);
				return $begin() + sz;
			}
			void _resize(size_t nsize) throws(NotEnoughSpaceException)
			{
				_realloc(nsize, maxt($size()*3/2, nsize, 4));
			}
			void _init(const Component* beg, size_t sz) throws(NotEnoughSpaceException)
			{
				_alloc(sz, sz);
				mema::ctor_copy($begin(), (InternalComponent*)beg, sz);
			}
			void _init(const Component* beg, const Component* end) throws(NotEnoughSpaceException)
			{
				_init(beg, end - (InternalComponent*)beg);
			}
			void _move(Component* beg, size_t sz) throws(NotEnoughSpaceException)
			{
				_alloc(sz, sz);
				mema::ctor_move($begin(), (InternalComponent*)beg, sz);
			}
			void _move(const Component* beg, size_t sz) throws(NotEnoughSpaceException)
			{
				_init(beg, sz);
			}
			ComponentRef* $_padding(size_t inc)
			{
				_assert(inc != 0);
				size_t sz = $size();
				_realloc(sz, sz + inc);
				return $end();
			}

			size_t $remaining() const noexcept
			{
				return $capacity() - $size();
			}
			bool emptyImpl() const noexcept
			{
				return $size() == 0;
			}
			InternalComponent* $end() noexcept
			{
				return $begin() + $size();
			}
			const InternalComponent* $end() const noexcept
			{
				return $begin() + $size();
			}
			InternalComponent* $limit() noexcept
			{
				return $begin() + $capacity();
			}
			const InternalComponent* $limit() const noexcept
			{
				return $begin() + $capacity();
			}

			constexpr WritableForm() = default;
			constexpr WritableForm(const WritableForm&) = default;
			constexpr WritableForm(WritableForm&& _move) = default;
			WritableForm(const Component* str, size_t len) noexcept 
			{
				_init(str, len);
			}
			WritableForm(const Component* str, const Component* end) noexcept
			{
				_init(str, end); 
			}
			WritableForm(std::initializer_list<InternalComponent> _list) noexcept
			{
				_init(_list.begin(), _list.size());
			}

		};
		template <size_t CAP, typename C, class Parent> 
		class BufferedForm :public Common<C, false, true, true, true, Parent>
		{
			CLASS_HEADER(BufferedForm, Common<C, false, true, true, true, Parent>);
		public:
			INHERIT_COMPONENT();

		private:
			size_t m_size;
			alignas(InternalComponent) byte m_buffer[sizeof(InternalComponent) * CAP];

		public:
			void _setSize(size_t nsize) noexcept
			{
				m_size = nsize;
			}
			void _setEnd(InternalComponentRef * _end)
			{
				m_size = _end - $begin();
			}
			void $_addEnd(size_t inc) noexcept
			{
				m_size += inc;
			}
			void _alloc(size_t nsize, size_t ncap) throws(NotEnoughSpaceException)
			{
				_assert(nsize <= ncap);
				if (ncap > CAP) throw NotEnoughSpaceException();
				_setSize(nsize);
			}
			void _realloc(size_t nsize, size_t ncap) throws(NotEnoughSpaceException)
			{
				_assert(nsize <= ncap);
				if (ncap > CAP) throw NotEnoughSpaceException();

				InternalComponent* old_end = $end();
				_setSize(nsize);
				InternalComponent* new_end = $end();
				if (old_end > new_end) mema::dtor(new_end, old_end);
			}
			void _clear() noexcept
			{
				_setSize(0);
			}
			void _shiftRight(size_t idx) throws(NotEnoughSpaceException)
			{
				size_t sz = $size();
				if (sz >= CAP) throw NotEnoughSpaceException();
				InternalComponent* axis = $begin() + idx;
				kr::mema::ctor_move_rd(axis + 1, axis, sz - idx);
				_setSize(sz + 1);
			}
			void _shiftRight(size_t idx, size_t count) throws(NotEnoughSpaceException)
			{
				size_t sz = $size();
				size_t nsz = sz + count;
				if (nsz > CAP) throw NotEnoughSpaceException();
				InternalComponent* axis = $begin() + idx;
				kr::mema::ctor_move_rd(axis + count, axis, sz - idx);
				_setSize(nsz);
			}
			void _reduce(size_t nsize) noexcept
			{
				size_t osize = $size();
				_assert(nsize <= osize);
				_setSize(osize);
			}

#pragma warning(push)
#pragma warning(disable: 26495)
			BufferedForm() noexcept
			{
				m_size = 0;
			}
#pragma warning(pop)
			~BufferedForm() noexcept
			{
				mema::dtor($begin(), $end());
			}
			BufferedForm(const BufferedForm & _copy)
			{
				_alloc(_copy.$size(), _copy.$size());
				mema::ctor_copy($begin(), _copy.$begin(), _copy.$size());
			}

			InternalComponent* $begin() noexcept
			{
				return (InternalComponent*)m_buffer;
			}
			const InternalComponent* $begin() const noexcept
			{
				return (InternalComponent*)m_buffer;
			}
			InternalComponent* $end() noexcept
			{
				return (InternalComponent*)m_buffer + m_size;
			}
			const InternalComponent* $end() const noexcept
			{
				return (InternalComponent*)m_buffer + m_size;
			}
			size_t $size() const noexcept
			{
				return m_size;
			}
			size_t $capacity() const noexcept
			{
				return CAP;
			}
			size_t sizeBytes() const noexcept
			{
				return m_size * sizeof(C);
			}
			size_t capacityBytes() const noexcept
			{
				return CAP * sizeof(C);
			}
			bool isNull() const noexcept
			{
				return false;
			}
			void reserve(size_t ncap) throws(NotEnoughSpaceException)
			{
				if ($capacity() < ncap) throw NotEnoughSpaceException();
			}

			using Data = BufferedData<CAP, C, Parent>;
		};;

		template <typename C, class Allocator, class Parent>
		class AllocatedForm : public Common<C, false, true, true, true, Parent>
		{
			CLASS_HEADER(AllocatedForm, Common<C, false, true, true, true, Parent>);
		public:
			INHERIT_COMPONENT();

		private:
			InternalComponent* m_begin;

		protected:
			static internal_component_t<C>* const null;
				
		public:
#define MINIMAL_EXPAND_SIZE ((32 + sizeof(InternalComponent) + 1) / sizeof(InternalComponent))

			void _setSize(size_t nsize) noexcept
			{
				((size_t*)m_begin)[-1] = nsize;
			}
			void _setEnd(InternalComponentRef * _end)
			{
				_setSize((InternalComponent*)_end - (InternalComponent*)m_begin);
			}
			void $_addEnd(size_t inc) noexcept
			{
				_setSize($size() + inc);
			}
			void _alloc(size_t $size, size_t cap) noexcept
			{
				_assert($size <= cap);
				if (cap == 0)
				{
					m_begin = null;
					return;
				}
				m_begin = Allocator::_mem_alloc(cap);
				_setSize($size);
			}
			void _realloc(size_t nsize, size_t ncap) throws(NotEnoughSpaceException)
			{
				_assert(nsize <= ncap);
				if (m_begin == null)
				{
					if (ncap == 0) return;
					m_begin = (InternalComponent*)Allocator::_mem_alloc(ncap);
				}
				else
				{
					if (Allocator::_mem_expand(m_begin, ncap))
					{
						if ($size() > nsize)
							mema::dtor(m_begin + nsize, $end());
					}
					else
					{
						size_t osize = $size();
						m_begin = (InternalComponent*)Allocator::_obj_move(m_begin, ncap,
							[&](InternalComponent * to) { mema::ctor_move_d(to, m_begin, osize); });
					}
				}
				_setSize(nsize);
			}
			void _free() noexcept
			{
				if (m_begin == null) return;
				Allocator::_mem_free(m_begin);
			}
			void _clear() noexcept
			{
				if (m_begin == null) return;
				_setSize(0);
			}
			void _shiftRight(size_t idx) throws(NotEnoughSpaceException)
			{
				size_t sz = $size();
				InternalComponent* beg = m_begin;
				if (beg != null)
				{
					InternalComponent* axis = beg + idx;
					if (Allocator::_mem_expand(m_begin, sz + 1))
					{
						kr::mema::ctor_move_rd(axis + 1, axis, sz - idx);
					}
					else
					{
						size_t cap = $capacity() * 3 / 2 + 1;
						if (cap < MINIMAL_EXPAND_SIZE) cap = MINIMAL_EXPAND_SIZE;
						m_begin = Allocator::_obj_move(m_begin, cap, [&](C * to) {
							mema::ctor_move_d(to, beg, idx);
							mema::ctor_move_d(to + idx + 1, axis, sz - idx);
						});
					}
				}
				else
				{
					m_begin = Allocator::_mem_alloc(MINIMAL_EXPAND_SIZE);
				}
				_setSize(sz + 1);
			}
			void _shiftRight(size_t idx, size_t count) throws(NotEnoughSpaceException)
			{
				size_t sz = $size();
				size_t nsz = sz + count;
				InternalComponent* beg = $begin();
				InternalComponent* axis = beg + idx;
				if (m_begin != null)
				{
					if (Allocator::_mem_expand(m_begin, nsz))
					{
						kr::mema::ctor_move_rd(axis + count, axis, sz - idx);
					}
					else
					{
						size_t cap = $capacity() * 3 / 2 + 1;
						if (cap < MINIMAL_EXPAND_SIZE) cap = MINIMAL_EXPAND_SIZE;
						if (cap < nsz) cap = nsz;
						m_begin = Allocator::_obj_move(m_begin, cap, [&](C * to) {
							mema::ctor_move_d(to, beg, idx);
							mema::ctor_move_d(to + idx + count, axis, sz - idx);
						});
					}
				}
				else
				{
					m_begin = Allocator::_mem_alloc(MINIMAL_EXPAND_SIZE);
				}
				_setSize(sz + count);
			}

			void* _basePointer() noexcept
			{
				return Allocator::_mem_base(m_begin);
			}

			AllocatedForm() noexcept
			{
				m_begin = null;
			}
			AllocatedForm(nullptr_t) noexcept
			{
				m_begin = null;
			}
			explicit AllocatedForm(size_t $size) noexcept
			{
				_alloc($size, $size);
				mema::ctor($begin(), $end());
			}
			AllocatedForm(size_t $size, size_t capacity) noexcept
			{
				_alloc($size, capacity);
				mema::ctor($begin(), $end());
			}
			AllocatedForm(const AllocatedForm & _copy)
			{
				_alloc(_copy.$size(), _copy.$size());
				mema::ctor_copy($begin(), _copy.$begin(), _copy.$size());
			}
			AllocatedForm(AllocatedForm && _move) noexcept
			{
				m_begin = _move.m_begin;
				_move.m_begin = null;
			}
			~AllocatedForm() noexcept
			{
				mema::dtor(m_begin, $end());
				_free();
			}

			const InternalComponent* $begin() const noexcept
			{
				return m_begin;
			}
			InternalComponent* $begin() noexcept
			{
				return m_begin;
			}
			const InternalComponent* $end() const noexcept
			{
				return m_begin + $size();
			}
			InternalComponent* $end() noexcept
			{
				return m_begin + $size();
			}
			size_t $size() const noexcept
			{
				return ((size_t*)m_begin)[-1];
			}
			size_t $capacity() const noexcept
			{
				if (m_begin == null) return 0;
				return Allocator::_mem_msize(m_begin);
			}
			size_t sizeBytes() const noexcept
			{
				return $size() * sizeof(C);
			}
			size_t capacityBytes() const noexcept
			{
				return _mem_msize_bytes(m_begin);
			}
			bool isNull() const noexcept
			{
				return m_begin == null;
			}
			void reserve(size_t ncap) throws(NotEnoughSpaceException)
			{
				size_t cap = $capacity();
				if (cap >= ncap) return;
				size_t sz = $size();
				_realloc(sz, ncap);
			}
			void shrink() noexcept
			{
				size_t osize = $size();
				if (osize == 0) return;
				size_t allocsize = Allocator::_mem_msize(m_begin);
				size_t usebytes = osize * sizeof(C);
				if (allocsize <= usebytes + 32) return;

				m_begin = Allocator::_obj_move(m_begin, usebytes, [&](C* to) {
					mema::ctor_move_d(to, m_begin, osize);
				});
				_setSize(osize);
			}

#undef MINIMAL_EXPAND_SIZE
		};
		template <typename C, class Allocator, class Parent>
		internal_component_t<C>* const AllocatedForm<C, Allocator, Parent>::null = (internal_component_t<C>*)(kr::_pri_::ZERO_MEMORY.buffer + 1);
			
	}
}
