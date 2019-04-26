#pragma once

#include <KR3/main.h>

namespace kr
{
	namespace ary
	{
		namespace data
		{
			template <typename C, size_t offset>
			class SAllocator
			{
			private:
				using InternalComponent = internal_component_t<C>;
				static constexpr size_t align = alignof(InternalComponent);

			public:
				static InternalComponent * _mem_alloc_bytes(size_t sz) noexcept
				{
					StackAllocator * allocator = StackAllocator::getInstance();
					if (align <= 1)
					{
						return (InternalComponent*)((byte*)allocator->allocate(sz + offset) + offset);
					}
					else if (offset == 0)
					{
						return (InternalComponent*)((byte*)allocator->allocate(sz, align));
					}
					else
					{
						return (InternalComponent*)((byte*)allocator->allocate(sz + offset, align, offset) + offset);
					}
				}
				static InternalComponent * _mem_alloc(size_t sz) noexcept
				{
					return _mem_alloc_bytes(sz * sizeof(InternalComponent));
				}
				static size_t _mem_msize_bytes(InternalComponent * p) noexcept
				{
					StackAllocator * allocator = StackAllocator::getInstance();
					return (allocator->msize((byte*)p - offset) - offset);
				}
				static size_t _mem_msize(InternalComponent * p) noexcept
				{
					return _mem_msize_bytes(p) / sizeof(InternalComponent);
				}
				static bool _mem_expand(InternalComponent * p, size_t sz) throws(NotEnoughSpaceException)
				{
					StackAllocator * allocator = StackAllocator::getInstance();
					if (allocator->isLastBlock((byte*)p - offset))
					{
						return allocator->expand((byte*)p - offset, sz * sizeof(InternalComponent) + offset);
					}
					else
					{
						if (_mem_msize_bytes(p) < sz * sizeof(InternalComponent))
							throw NotEnoughSpaceException();
						return true;
					}
				}
				static void _mem_reduce(InternalComponent * p, size_t sz) noexcept
				{
					_assert(sz <= _mem_msize(p));
					StackAllocator * allocator = StackAllocator::getInstance();
					if (!allocator->isLastBlock((byte*)p - offset)) return;
					allocator->expand((byte*)p - offset, sz * sizeof(InternalComponent) + offset);
				}
				static void _mem_free(InternalComponent * p) noexcept
				{
					StackAllocator * allocator = StackAllocator::getInstance();
					allocator->free((byte*)p - offset);
				}

				template <typename LAMBDA>
				static InternalComponent* _obj_move(InternalComponent * from, size_t ncap, const LAMBDA & ctor_move_d) throws(NotEnoughSpaceException)
				{
					StackAllocator * allocator = StackAllocator::getInstance();
					byte* allocpoint = (byte*)from - offset;
					if (!allocator->isLastBlock(allocpoint))
						throw NotEnoughSpaceException();

					ncap *= sizeof(InternalComponent);

					StackAllocator::Node * node = allocator->getLastNode();
					InternalComponent* narr = (InternalComponent*)((byte*)allocator->allocateWithNewNode(ncap, align, offset) + offset);
					ctor_move_d(narr);
					node->free(allocpoint);
					return narr;
				}
			};
			template <typename C, size_t offset>
			class DAllocator
			{
			private:
				using InternalComponent = internal_component_t<C>;
				static constexpr size_t align = alignof(InternalComponent);
				using allocator = alloc<align, offset>;

			public:
				static InternalComponent * _mem_alloc_bytes(size_t sz) noexcept
				{
					return (InternalComponent*)((byte*)allocator::allocate(sz + offset) + offset);
				}
				static InternalComponent * _mem_alloc(size_t sz) noexcept
				{
					return _mem_alloc_bytes(sz * sizeof(InternalComponent));
				}
				static size_t _mem_msize_bytes(InternalComponent * p) noexcept
				{
					return allocator::msize((byte*)p - offset);
				}
				static size_t _mem_msize(InternalComponent * p) noexcept
				{
					return _mem_msize_bytes(p) / sizeof(InternalComponent);
				}
				static bool _mem_expand(InternalComponent * p, size_t sz) noexcept
				{
					return allocator::expand((byte*)p - offset, sz * sizeof(InternalComponent) + offset);
				}
				static void _mem_reduce(InternalComponent * p, size_t sz) noexcept
				{
					_assert(sz <= _mem_msize(p));
				}
				static void _mem_free(InternalComponent * p) noexcept
				{
					return allocator::free((byte*)p - offset);
				}

				template <typename LAMBDA>
				static InternalComponent* _obj_move(InternalComponent * from, size_t ncap, const LAMBDA & ctor_move_d) noexcept
				{
					InternalComponent* narr = _mem_alloc(ncap);
					ctor_move_d(narr);
					_mem_free(from);
					return narr;
				}
			};
		}
	}
}