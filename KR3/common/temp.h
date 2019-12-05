#pragma once

#include "memlib.h"

namespace kr
{
	class StackAllocator
	{
	public:
		struct AllocHead
		{
			AllocHead * next;
			AllocHead * previous;
			byte * usefrom;
			byte * useto;

			static AllocHead * get(void * p) noexcept;
		};

		struct Node
		{
			Node * prev;
			AllocHead axis;

			ATTR_NO_DISCARD static Node * create(Node * pnode, size_t sz) noexcept;
			ATTR_NO_DISCARD byte * end() noexcept;
			ATTR_NO_DISCARD size_t size() noexcept;
			ATTR_NO_DISCARD bool empty() noexcept;
			ATTR_NO_DISCARD autoptr allocate(size_t sz) noexcept;
			ATTR_NO_DISCARD autoptr allocate(size_t sz, size_t align) noexcept;
			ATTR_NO_DISCARD autoptr allocate(size_t sz, size_t align, size_t offset) noexcept;
			void free(void * data) noexcept;
			ATTR_NO_DISCARD bool expand(void * data, size_t sz) noexcept;
			ATTR_NO_DISCARD size_t msize(void * data) noexcept;

		private:
			ATTR_NO_DISCARD autoptr _allocate(AllocHead * head, byte * usefrom, byte * useto) noexcept;
			ATTR_NO_DISCARD bool _expandTo(byte* to) noexcept;
		};

		StackAllocator(size_t reserve = 1024) noexcept;
		~StackAllocator() noexcept;
		void terminate() noexcept;
		ATTR_NO_DISCARD autoptr allocate(size_t sz) noexcept;
		ATTR_NO_DISCARD autoptr allocate(size_t sz, size_t align) noexcept;
		ATTR_NO_DISCARD autoptr allocate(size_t sz, size_t align, size_t offset) noexcept;
		ATTR_NO_DISCARD autoptr allocateWithNewNode(size_t sz) noexcept;
		ATTR_NO_DISCARD autoptr allocateWithNewNode(size_t sz, size_t align) noexcept;
		ATTR_NO_DISCARD autoptr allocateWithNewNode(size_t sz, size_t align, size_t offset) noexcept;
		bool expand(void * data, size_t sz) noexcept;
		ATTR_NO_DISCARD size_t msize(void * data) noexcept;
		ATTR_NO_DISCARD Node * getLastNode() noexcept;
		void free(void * data) noexcept;
		ATTR_NO_DISCARD bool empty() noexcept;
		ATTR_NO_DISCARD bool isLastBlock(void * block) noexcept;

		ATTR_NO_DISCARD static StackAllocator * getInstance() noexcept;

	private:
		ATTR_NO_DISCARD Node * _allocateNewNode(size_t need) noexcept;

		Node * m_last;
		size_t m_fullsize; // node size + reserve
		size_t m_reserve;
		void * m_lastBlock;
	};

	enum TempExtendAllocate_t { TempExtendAllocate };

	template <typename T> class Temp
	{
	public:
		Temp(TempExtendAllocate_t, size_t sz)
		{
			m_data = (T*)StackAllocator::getInstance()->allocate(sz, alignof(T));
			new(m_data) T;
		}
		template<typename ... ARGS>
		Temp(const ARGS & ... args)
		{
			m_data = (T*)StackAllocator::getInstance()->allocate(sizeof(T), alignof(T));
			new(m_data) T(args ...);
		}
		~Temp()
		{
			StackAllocator::getInstance()->free(m_data);
		}

		Temp(const Temp&) = delete;
		Temp& operator =(const Temp&) = delete;

		Temp(Temp&& _move) noexcept
		{
			m_data = _move.m_data;
			_move.m_data = nullptr;
		}
		Temp& operator =(Temp&& _move) noexcept
		{
			this->~Temp();
			new(this) Temp(move(_move));
			return *this;
		}

		T* operator ->() const
		{
			return m_data;
		}
		operator T*() const
		{
			return m_data;
		}

	private:
		T * m_data;
	};
}

