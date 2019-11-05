#include "stdafx.h"
#include "alloc.h"
#include "llutil/dout.h"

#include <KR3/mt/criticalsection.h>
#include <KR3/util/keeper.h>
#include <unordered_map>
#include <malloc.h>

#pragma warning(disable:4074)
#pragma init_seg(compiler)

using namespace kr;

#ifndef _MSC_VER

#define _NORMAL_BLOCK 0

size_t _msize(void * data) noexcept
{
	return malloc_usable_size(data);
}
void * ___expand_unuse(void * data, size_t size) noexcept
{
	size_t oldsize = malloc_usable_size(data);
	if (oldsize >= size) return data;
	return nullptr;
	//size_t extra = size - oldsize;

	//__heap_lock(&__malloc_heap_lock);
	//extra = __heap_alloc_at(&__malloc_heap, base_mem + size, extra);
	//__heap_unlock(&__malloc_heap_lock);

	//if (extra)
	//	/* Record the changed size.  */
	//	MALLOC_SET_SIZE(base_mem, size + extra);
}
void * _aligned_offset_malloc(size_t size, size_t alignment, size_t offset) noexcept
{
	_assert(((alignment - 1) & alignment) == 0);

	size_t extra = alignment + offset + sizeof(void*) - alignof(max_align_t);
	void * origin = malloc(size + extra);
	if (origin == nullptr) return nullptr;
	
	void ** data = (void**)((((uintptr_t)origin + extra) & ((uintptr_t)-1 ^ (alignment - 1))) - offset);
	data[-1] = origin;
	return data;
}
void * _aligned_malloc(size_t size, size_t alignment) noexcept
{
	return _aligned_offset_malloc(size, alignment, 0);
}
void _aligned_free(void * data) noexcept
{
	if (data == nullptr)
		return;
	free(((void**)data)[-1]);
}
size_t _aligned_msize(void * data, size_t alignment, size_t offset) noexcept
{
	if (data == nullptr)
		return 0;
	void * origin = ((void**)data)[-1];
	return malloc_usable_size(origin) + (uintptr_t)origin - (uintptr_t)data;
}
#endif

#ifndef NDEBUG

#ifdef _MSC_VER

constexpr size_t nNoMansLandSize = 4;
constexpr unsigned char NoMansLand = 0xFD;
constexpr uint32_t NoMansLand32 = NoMansLand | (NoMansLand << 8) | (NoMansLand << 16) | (NoMansLand << 24);
constexpr uint32_t AMALang32 = 0xEDEDEDED;
typedef struct _CrtMemBlockHeader
{
	// Pointer to the block allocated just before this one:  
	struct _CrtMemBlockHeader *pBlockHeaderNext;
	// Pointer to the block allocated just after this one:  
	struct _CrtMemBlockHeader *pBlockHeaderPrev;
	char *szFileName;    // File name  
	int nLine;           // Line number  
	int nBlockUse;       // Type of block  
	size_t nDataSize;    // Size of user block  
	long lRequest;       // Allocation number  
						 // Buffer just before (lower than) the user's memory:  
	unsigned char gap[nNoMansLandSize];
	
	static _CrtMemBlockHeader* from(void * data) noexcept
	{
		switch (((uint32_t*)data)[-1])
		{
		case AMALang32:
			_assert(sizeof(void*) != 8);
			data = (void*)(uintptr_t)((uint32_t*)data)[-2];
			break;
		case NoMansLand32:
			break;
		default:
			debug(); // invalid memory block
			return nullptr;
		}
		_CrtMemBlockHeader* block = (_CrtMemBlockHeader*)data - 1;
		_assert(*(uint32_t*)block->gap == NoMansLand32);
		byte * gap2 = (byte*)data + block->nDataSize;
		_assert(*(uint32_t*)gap2 == NoMansLand32);
		return block;
	}
} _CrtMemBlockHeader;

/* In an actual memory block in the debug heap,
* this structure is followed by:
*   unsigned char data[nDataSize];
*   unsigned char anotherGap[nNoMansLandSize];
*/

#elif defined(_EMSCRIPTEN__)
#include <emscripten.h>
#else

#endif

namespace
{
	int s_allocCounter = 0;
	int s_allocCounterBreak = 0;
	bool s_allocCounterBreakEnabled = false;
	atomic<bool> s_memoryObserverDisabled;

	class BlockInfo;
	class MemoryObserver;
#ifdef _MSC_VER
	constexpr uint32_t KEN_MARK = '\0NEK';
#endif
}

namespace
{
#ifdef _MSC_VER
	_CRT_ALLOC_HOOK s_oldHook;

	int allocHook(int nAllocType, void *pvData,
		size_t nSize, int nBlockUse, long lRequest,
		const unsigned char * szFileName, int nLine) noexcept;

#endif
	
	class BlockInfo
	{
		friend MemoryObserver;

	public:
		void checkAligned(bool aligned) noexcept;
		void setAddress(void * data) noexcept;
		void * getAddress() noexcept;
		static BlockInfo * from(void * data) noexcept;
		static void create(void * data, bool aligned) noexcept;

	private:
		BlockInfo() = default;
		~BlockInfo() = default;
#ifdef _MSC_VER
		uint32_t m_kenMark;
#endif
		const char * m_filename;
		void * m_address;
		int m_line;
		int m_allocationNumber;
		bool m_isAligned;
		BlockInfo * m_next;
		BlockInfo * m_prev;

	};

#ifdef _MSC_VER
	AtomicBufferKeeper<sizeof(BlockInfo), 512> s_blockInfoKeeped;
#endif

	struct MemoryCloser
	{
		MemoryCloser() noexcept
		{
#ifdef _MSC_VER
			// _assert(!_CrtDumpMemoryLeaks()); // obs용 플러그인 dll에 사용중, 지나치게 많은 메모리 덤프가 발견되었다. 이유 불명
#endif
		}
	};
	class MemoryObserver:public MemoryCloser
	{
	private:
		BlockInfo s_axis;
		CriticalSection m_cs;

	public:
		MemoryObserver() noexcept
		{
#ifdef _MSC_VER
			s_oldHook = _CrtSetAllocHook(allocHook);
#else
#endif
			s_axis.m_filename = nullptr;
			s_axis.m_line = -1;
			s_axis.m_allocationNumber = -1;
			s_axis.m_isAligned = false;
			s_axis.m_next = s_axis.m_prev = &s_axis;
		}
		~MemoryObserver() noexcept
		{
#ifdef _MSC_VER
			_CrtSetAllocHook(s_oldHook);
#endif
			test();
#ifdef _MSC_VER
			_assert(_CrtCheckMemory()); // 메모리가 오염되었다
#endif
			_assert(empty()); // KR 라이브러리에 감지에 의한 메모리 누수가 발견되었다
		}

		struct Iterator
		{
			BlockInfo * p;

			bool operator != (const Iterator & other) const noexcept
			{
				return p != other.p;
			}
			Iterator & operator ++() noexcept
			{
				p = p->m_next;
				return *this;
			}
			BlockInfo & operator *() noexcept
			{
				return *p;
			}
		};

		Iterator begin() noexcept
		{
			return { s_axis.m_next };
		}
		Iterator end() noexcept
		{
			return { &s_axis };
		}

		void test() noexcept
		{
			kr::LLDebugOutput dout;

			m_cs.enter();
			for (BlockInfo & info : *this)
			{
				dout << info.m_filename << '(' << info.m_line << ") //" << info.m_allocationNumber << "\r\n";

				uintptr_t value = (uintptr_t)info.m_address;
				dout << "Address: 0x";
				for (int i = 0; i < sizeof(uintptr_t) * 2; i++)
				{
					value = kr::intrinsic<sizeof(uintptr_t)>::rotl(value, 4);
					uintptr_t v = value & 0xf;
					if (v < 10)
						dout << (char)(v + '0');
					else
						dout << (char)(v - 10 + 'A');
				}
				dout << "\r\n";
			}
			m_cs.leave();
		}
		bool empty() noexcept
		{
			CsLock __lock = m_cs;
			return s_axis.m_next == &s_axis;
		}
		void clear() noexcept
		{
#ifdef _MSC_VER
			BlockInfo * willFree = nullptr;
#endif
			m_cs.enter();
			while (!empty())
			{
				BlockInfo * block = s_axis.m_next;
				detachBlock(block);
#ifdef _MSC_VER
				block->m_next = willFree;
				willFree = block;
#endif
			}
			m_cs.leave();

#ifdef _MSC_VER
			while (willFree != nullptr)
			{
				BlockInfo * next = willFree->m_next;
				s_blockInfoKeeped.rawFree(willFree);
				willFree = next;
			}
#endif
		}
		void attachBlock(BlockInfo * info, bool aligned) noexcept
		{
			m_cs.enter();
#ifdef _MSC_VER
			info->m_kenMark = KEN_MARK;
#endif
			if (s_allocCounterBreakEnabled && s_allocCounter == s_allocCounterBreak)
			{
				debug();
			}
			info->m_allocationNumber = s_allocCounter++;
			info->m_filename = "?";
			info->m_line = -1;
			info->m_isAligned = aligned;

			s_axis.m_prev->m_next = info;
			info->m_prev = s_axis.m_prev;
			info->m_next = &s_axis;
			s_axis.m_prev = info;
			m_cs.leave();
		}
		void removeBlock(BlockInfo * block) noexcept
		{
			m_cs.enter();
			detachBlock(block);
			m_cs.leave();
#ifdef _MSC_VER
			s_blockInfoKeeped.rawFree(block);
#endif
		}
		void detachBlock(BlockInfo * block) noexcept
		{
#ifdef _MSC_VER
			_CrtMemBlockHeader::from(block->getAddress())->szFileName = nullptr;
#else
			block->setAddress(nullptr);
#endif

			block->m_next->m_prev = block->m_prev;
			block->m_prev->m_next = block->m_next;

			block->m_next = block->m_prev = nullptr;
		}
		void check() noexcept
		{
			m_cs.enter();
			for (BlockInfo & info : *this)
			{
			}
			m_cs.leave();
#ifdef _MSC_VER
			_assert(_CrtCheckMemory());
#endif
		}
		void reline(void *data, const char * file, int line) noexcept
		{
			if (data == nullptr) return;
			BlockInfo * block = BlockInfo::from(data);
			if (block == nullptr) return;
			block->m_filename = file;
			block->m_line = line;
		}
		void depend(void * parent, void * child) noexcept
		{
		}
		void checkAligned(void * data, bool aligned) noexcept
		{
			if (s_memoryObserverDisabled) return;
			if (data == nullptr) return;
			BlockInfo * block = BlockInfo::from(data);
			if (block == nullptr) return;
			_assert(block->m_isAligned == aligned);
		}
	};

	MemoryObserver s_mo;


	void BlockInfo::checkAligned(bool aligned) noexcept
	{
		_assert(m_isAligned == aligned);
	}
	void BlockInfo::setAddress(void * data) noexcept
	{
		m_address = data;
	}
	void * BlockInfo::getAddress() noexcept
	{
		return m_address;
	}
	BlockInfo * BlockInfo::from(void * data) noexcept
	{
#ifdef _MSC_VER
		const char * filename = _CrtMemBlockHeader::from(data)->szFileName;
		if (filename == nullptr) return nullptr;
		BlockInfo * info = (BlockInfo*)filename;
		if (info->m_kenMark != KEN_MARK) return nullptr;
		return info;
#else
		BlockInfo * info = (BlockInfo*)data - 1;
		if (info->getAddress() == nullptr) return nullptr;
		return info;
#endif
	}
	void BlockInfo::create(void * data, bool aligned) noexcept
	{
		_assert(((uintptr_t)data % alignof(BlockInfo)) == 0);

#ifdef _MSC_VER
		if (s_memoryObserverDisabled) return;
		_CrtMemBlockHeader * header = _CrtMemBlockHeader::from(data);
		_assert(header->szFileName == nullptr);
		BlockInfo * info = (BlockInfo *)s_blockInfoKeeped.rawAlloc();
		header->szFileName = (char*)info;
#else
		BlockInfo * info = (BlockInfo*)data - 1;
		if (s_memoryObserverDisabled)
		{
			info->setAddress(nullptr);
			return;
		}
#endif
		s_mo.attachBlock(info, aligned);
		info->setAddress(data);
	}
}


#ifdef _MSC_VER


namespace
{
	int allocHook(int nAllocType, void *pvData,
		size_t nSize, int nBlockUse, long lRequest,
		const unsigned char * szFileName, int nLine) noexcept
	{
		switch (nAllocType)
		{
		case _HOOK_ALLOC:
			break;
		case _HOOK_FREE:
			szFileName = (const unsigned char *)_CrtMemBlockHeader::from(pvData)->szFileName;
			// get a pointer to memory block header

			if (szFileName != nullptr && *(uint32_t*)szFileName == KEN_MARK)
			{
				BlockInfo* block = (BlockInfo*)szFileName;
				s_mo.removeBlock(block);
			}
			break;
		case _HOOK_REALLOC:
			break;
		}
		return s_oldHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
	}
}
#endif

void kr::memtest_set_axis() noexcept
{
	if (s_memoryObserverDisabled) return;
	s_mo.clear();
}
void kr::memtest() noexcept
{
	if (s_memoryObserverDisabled) return;
	if(s_mo.empty()) return;
	s_mo.test();
	debug(); // 메모리가 누수되었다. 출력(Output)을 참조.
}
void kr::memcheck() noexcept
{
	if (s_memoryObserverDisabled) return;
	s_mo.check();
}
void kr::_pri_::reline_new_impl(void * ptr, const char * file, int line) noexcept
{
	if (s_memoryObserverDisabled) return;
	if(ptr == nullptr) return;
	s_mo.reline(ptr, file, line);
}
void kr::_pri_::depend_new_impl(void * parent, void * child) noexcept
{
	if (s_memoryObserverDisabled) return;
	if (parent == nullptr) return;
	if (child == nullptr) return;

	s_mo.depend(parent, child);
}

#else

void kr::memtest_set_axis() noexcept
{
}
void kr::memtest() noexcept
{
}
void kr::memcheck() noexcept
{
}

#endif

void* kr::_pri_::alloc_base<0, 0>::allocate(size_t sz) noexcept
{
	return krmalloc(sz);
}
void kr::_pri_::alloc_base<0, 0>::free(void * data) noexcept
{
	krfree(data);
}
bool kr::_pri_::alloc_base<0, 0>::expand(void * data, size_t nsize) noexcept
{
#ifdef _MSC_VER
	if (nsize <= msize_impl(data)) return true;
	void * expanded = _expand_dbg(data, nsize, _NORMAL_BLOCK, (char*)BlockInfo::from(data), 0);
	_assert(expanded == nullptr || expanded == data);
	return expanded != nullptr;
#else
	return nsize <= msize_impl(data);
#endif
}
size_t kr::_pri_::alloc_base<0, 0>::msize(const void * data) noexcept
{
#ifndef NDEBUG
	s_mo.checkAligned((void*)data, false);
#endif
	return msize_impl(data);
}

void* kr::_pri_::aligned_alloc_impl(size_t sz, size_t aligned) noexcept
{
	_assert(sz < 0xffffffff);
#if defined(_MSC_VER) || defined(NDEBUG)
	void * ptr = _aligned_malloc(sz, aligned);
	if (ptr == nullptr) notEnoughMemory();
#else
	void * ptr = _aligned_offset_malloc(sz + sizeof(BlockInfo), aligned, sizeof(BlockInfo));
	if (ptr == nullptr) notEnoughMemory();
	(byte&)ptr += sizeof(BlockInfo);
#endif
#ifndef NDEBUG
	BlockInfo::create(ptr, true);
#endif
	return ptr;
}
void* kr::_pri_::aligned_alloc_impl(size_t sz, size_t aligned, size_t offset) noexcept
{
	_assert(sz < 0xffffffff);
#if defined(_MSC_VER) || defined(NDEBUG)
	void * ptr = _aligned_offset_malloc(sz, aligned, offset);
	if (ptr == nullptr) notEnoughMemory();
#else
	void * ptr = _aligned_offset_malloc(sz + sizeof(BlockInfo), aligned, offset + sizeof(BlockInfo));
	if (ptr == nullptr) notEnoughMemory();
	(byte&)ptr += sizeof(BlockInfo);
#endif
#ifndef NDEBUG
	BlockInfo::create(ptr, true);
#endif
	return ptr;
}
bool kr::_pri_::aligned_expand_impl(void * data, size_t nsize, size_t aligned, size_t offset) noexcept
{
#if defined(_MSC_VER) || defined(NDEBUG)
	return nsize <= _aligned_msize(data, aligned, offset);
#else
	return nsize <= _aligned_msize((byte*)data - sizeof(BlockInfo), aligned, offset);
#endif
}
size_t kr::_pri_::aligned_msize_impl(const void * ptr, size_t aligned, size_t offset) noexcept
{
#ifndef NDEBUG
	s_mo.checkAligned((void*)ptr, true);
#endif
#if defined(_MSC_VER) || defined(NDEBUG)
	return _aligned_msize((void*)ptr, aligned, offset);
#else
	return _aligned_msize((byte*)ptr - sizeof(BlockInfo), aligned, offset);
#endif
}
void kr::_pri_::aligned_free_impl(void * ptr) noexcept
{
#if defined(_MSC_VER) || defined(NDEBUG)
	_aligned_free(ptr);
#else
	BlockInfo * block = BlockInfo::from(ptr);
	if (block != nullptr) s_mo.removeBlock(block);
	_aligned_free(block);
#endif
}

void * kr::_pri_::krmalloc(size_t size) noexcept
{
	_assert(size < 0xffffffff);
#if defined(_MSC_VER) || defined(NDEBUG)
	void * ptr = malloc(size);
	if (ptr == nullptr)
	{
		notEnoughMemory();
	}
#ifndef NDEBUG
	BlockInfo::create(ptr, false);
#endif
#else
	void * ptr = malloc(size + sizeof(BlockInfo));
	if (ptr == nullptr)
	{
		notEnoughMemory();
	}
	(byte&)ptr += sizeof(BlockInfo);
	BlockInfo::create(ptr, false);
#endif
	return ptr;
}
void kr::_pri_::krfree(void * ptr) noexcept
{
#if defined(_MSC_VER) || defined(NDEBUG)
	free(ptr);
#else
	BlockInfo * block = BlockInfo::from(ptr);
	if (block != nullptr) s_mo.removeBlock(block);
	free(block);
#endif
}
size_t kr::_pri_::msize_impl(const void * data) noexcept
{
#if defined(_MSC_VER) || defined(NDEBUG)
	return _msize((void*)data);
#else
	return _msize((byte*)data - sizeof(BlockInfo));
#endif
}

#ifdef NDEBUG

void kr::disableKrMemoryObserver(bool disabled) noexcept
{
}
void kr::setAllocCounterBreak(int counter) noexcept
{
}

#else

#ifndef _MSC_VER
void* operator new(size_t sz)
{
	return krmalloc(size);
}
void operator delete(void* p)
{
	krfree(p);
}
#endif

void kr::disableKrMemoryObserver(bool disabled) noexcept
{
	s_memoryObserverDisabled = disabled;
	if (disabled)
	{
		s_mo.clear();
	}
}
void kr::setAllocCounterBreak(int counter) noexcept
{
	s_allocCounterBreak = counter;
	s_allocCounterBreakEnabled = true;
}

#endif
