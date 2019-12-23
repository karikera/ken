#pragma once

#include "dataform.h"
#include "allocator.h"

namespace kr
{
	namespace ary
	{
		template <size_t CAP, typename C, class Parent> 
		class BufferedData: public WritableForm<BufferedForm<CAP, C, Parent>, BufferedData<CAP, C, Parent>>
		{
			using Super = WritableForm<BufferedForm<CAP, C, Parent>, BufferedData<CAP, C, Parent>>;
		public:
			using Super::Super;
			using Data = BufferedData;
			using Self = Wrap<BufferedData>;

			constexpr BufferedData() = default;
			constexpr BufferedData(const BufferedData&) = default;
			constexpr BufferedData(BufferedData&&) = default;
		};
		template <typename C, class Parent> 
		class AllocatedData : public WritableForm<AllocatedForm<C, DAllocator<C, sizeof(size_t)>, Parent>, AllocatedData<C, Parent>>
		{
			using Super = WritableForm<AllocatedForm<C, DAllocator<C, sizeof(size_t)>, Parent>, AllocatedData<C, Parent>>;
		public:
			using Super::Super;
			using Data = AllocatedData;
			using Self = Wrap<AllocatedData>;

			using Super::$begin;
			using Super::$size;
			using Super::$capacity;

			using Super::_setSize;
			using Super::_realloc;
			using Super::_alloc;
			using Super::_basePointer;

			constexpr AllocatedData() = default;
			constexpr AllocatedData(const AllocatedData&) = default;
			constexpr AllocatedData(AllocatedData&&) = default;
			friend void * getAllocatedPointer(AllocatedData & data) noexcept
			{
				if(data.isNull()) return nullptr;
				return (void*)((size_t*)data.$begin() - 1);
			}
		};
		template <typename C, class Parent> 
		class TemporaryData : public WritableForm<AllocatedForm<C, SAllocator<C, sizeof(size_t)>, Parent>, TemporaryData<C, Parent>>
		{
			using Super = WritableForm<AllocatedForm<C, SAllocator<C, sizeof(size_t)>, Parent>, TemporaryData<C, Parent>>;
		public:
			using Super::Super;
			using Data = TemporaryData;
			using Self = Wrap<TemporaryData>;

			constexpr TemporaryData() = default;
			TemporaryData(const TemporaryData &) = delete;
			constexpr TemporaryData(TemporaryData && _move) = default;
		};
		template <typename C, class Parent>
		class ReadableData:public Common<C, true, true, false, false, Parent>
		{
			CLASS_HEADER(ReadableData, Common<C, true, true, false, false, Parent>);
		public:
			INHERIT_COMPONENT();

		private:
			const InternalComponent* m_begin;
			const InternalComponent* m_end;

		public:

			template <typename T>
			void print(const T & v) = delete;

			void setBegin(const InternalComponent* _beg) noexcept
			{
				m_begin = _beg;
			}
			void addBegin(intptr_t n) noexcept
			{
				m_begin += n;
			}
			void setEnd(const InternalComponent* end) noexcept
			{
				m_end = end;
			}
			void addEnd(intptr_t n) noexcept
			{
				m_end += n;
			}
			void _init(const C* beg, size_t sz) noexcept
			{
				m_begin = (InternalComponent*)beg;
				m_end = (InternalComponent*)beg + sz;
			}
			void _init(const C* beg, const C * end) noexcept
			{
				m_begin = (InternalComponent*)beg;
				m_end = end;
			}

			const InternalComponent* $begin() const noexcept
			{
				return m_begin;
			}
			const InternalComponent* $end() const noexcept
			{
				return m_end;
			}
			size_t $size() const noexcept
			{
				_assert((m_begin == nullptr) == (m_end == nullptr));
				return ((byte*)m_end - (byte*)m_begin) / sizeof(InternalComponent);
			}
			bool emptyImpl() const noexcept
			{
				return m_begin == m_end;
			}
			bool isNull() const noexcept
			{
				return m_begin == nullptr;
			}

			// InStream
			const C* $read(size_t *_len) throws(EofException)
			{
				const InternalComponent* out = (InternalComponent*)m_begin;
				if (out == m_end)
					throw EofException();

				size_t reqsize = *_len;
				size_t remaining = m_end - out;
				if (reqsize > remaining)
					reqsize = *_len = remaining;
				m_begin += reqsize;
				return (C*)out;
			}

			using Data = ReadableData;
			using Self = Wrap<ReadableData>;

			constexpr ReadableData() = default;
			constexpr ReadableData(const ReadableData&) = default;
#pragma warning(push)
#pragma warning(disable: 26495)
			ReadableData(nullptr_t) noexcept : m_begin(nullptr) {}
#pragma warning(pop)
			ReadableData(const zerolen_t&) noexcept : m_begin(nullptr), m_end(nullptr) {}
			constexpr ReadableData(const C* str, size_t _len) noexcept
				:m_begin((InternalComponent*)str), m_end((InternalComponent*)str + _len)
			{
			}
			constexpr ReadableData(const C* str, const C* end) noexcept
				: m_begin((InternalComponent*)str), m_end((InternalComponent*)end)
			{
			}
			constexpr ReadableData(std::initializer_list<InternalComponent> _list) noexcept 
				: ReadableData(_list.begin(), _list.end())
			{
			}

			template <size_t SIZE>
			static Self fromArray(InternalComponent(&buffer)[SIZE]) noexcept
			{
				KR_DEFINE_MMEM();
				InternalComponent chr = (InternalComponent)'\0';
				InternalComponent * end = memm::find(buffer, chr, SIZE);
				return Self((C*)buffer, (C*)(end == nullptr ? buffer +SIZE : end));
			}
			template <typename T>
			static Self fromAny(const T & any) noexcept
			{
				return Self((C*)&any, (C*)((byte*)any + sizeof(T)));
			}
		};
		template <typename C, class Parent> 
		class AccessableData :public Common<C, true, true, false, true, Parent>
		{
			CLASS_HEADER(AccessableData, Common<C, true, true, false, true, Parent>);
		public:
			INHERIT_COMPONENT();

		private:
			InternalComponent* m_begin;
			InternalComponent* m_end;

		public:

			template <typename T>
			void print(const T & v) = delete;

			void setBegin(InternalComponent* _beg) noexcept
			{
				m_begin = _beg;
			}
			void addBegin(intptr_t n) noexcept
			{
				m_begin += n;
			}
			void setEnd(InternalComponent* end) noexcept
			{
				m_end = end;
			}
			void _init(C* beg, size_t sz) noexcept
			{
				m_begin = (InternalComponent*)beg;
				m_end = (InternalComponent*)beg + sz;
			}
			void _init(C* beg, const C * end) noexcept
			{
				m_begin = (InternalComponent*)beg;
				m_end = (InternalComponent*)end;
			}

			InternalComponent* $begin() const noexcept
			{
				return m_begin;
			}
			InternalComponent* $end() const noexcept
			{
				return m_end;
			}
			size_t $size() const noexcept
			{
				return ((byte*)m_end - (byte*)m_begin) / sizeof(InternalComponent);
			}
			bool emptyImpl() const noexcept
			{
				return m_begin == m_end;
			}
			bool isNull() const noexcept
			{
				return m_begin == nullptr;
			}

			// InStream
			C* $read(size_t *_len) throws(EofException)
			{
				const C* out = m_begin;
				if (out + *_len > m_end) throw EofException();
				m_begin += *_len;
				return out;
			}

			using Data = AccessableData;
			using Self = Wrap<AccessableData>;

			AccessableData() noexcept {}
			AccessableData(nullptr_t) noexcept : m_begin(nullptr) {}
			AccessableData(C* str, size_t _len) noexcept { _init(str, _len); }
			AccessableData(C* str, C* end) noexcept { _init(str, end); }
			template <size_t sz>
			AccessableData(InternalComponent(&_array)[sz]) noexcept { _init(_array, sz); }
		};
		template <typename C, class Parent> 
		class WritableData:public Common<C, false, false, true, true, Parent>
		{
			CLASS_HEADER(WritableData, Common<C, false, false, true, true, Parent>);
		public:
			INHERIT_COMPONENT();

		private:
			InternalComponent* m_end;
			InternalComponent* m_limit;

		public:
			void _setEnd(InternalComponentRef* str) noexcept
			{
				m_end = (InternalComponent*)str;
			}
			void $_addEnd(size_t inc) noexcept
			{
				m_end += inc;
			}
			void _setLimit(InternalComponentRef* str) noexcept
			{
				m_limit = (InternalComponent*)str;
			}
			InternalComponentRef* $_extend(size_t inc) throws(NotEnoughSpaceException)
			{
				InternalComponent* out = m_end;
				InternalComponent* nptr = out + inc;
				if (nptr > m_limit) throw NotEnoughSpaceException();
				m_end = nptr;
				return out;
			}
			InternalComponentRef* $_padding(size_t inc) throws(NotEnoughSpaceException)
			{
				if (m_end + inc > m_limit) throw NotEnoughSpaceException();
				return m_end;
			}
			void _init() = delete;

			InternalComponentRef* $end() const noexcept
			{
				return m_end;
			}
			size_t $remaining() const noexcept
			{
				return m_limit-m_end;
			}
			InternalComponentRef* $limit() const noexcept
			{
				return m_limit;
			}
			bool isNull() const noexcept
			{
				return m_end == nullptr;
			}
			Ref flip(C* beg) noexcept
			{
				return Ref(beg, m_end);
			}


			using Data = WritableData;
			using Self = Wrap<WritableData>;

			bool equals(Ref ref) const noexcept
			{
				return false;
			}

			WritableData() noexcept {}
			WritableData(nullptr_t) noexcept : m_end(nullptr) {}
			WritableData(C* str, size_t _len) noexcept : m_end((InternalComponent*)str), m_limit(str + _len) {}
			WritableData(C* str, C* end) noexcept : m_end((InternalComponent*)str), m_limit((InternalComponent*)end) {}
			template <size_t _size> WritableData(InternalComponent(&buffer)[_size]) noexcept : WritableData(buffer, _size) {}
			template <size_t _size, class _Parent> WritableData(const BufferedData<_size, C, _Parent>& data) noexcept
				:WritableData(data.$end(), data.$limit())
			{
			}
		};
	}
	namespace _pri_
	{
#ifndef NDEBUG
		template <typename C, class Parent>
		void reline_new_impl(ary::AllocatedData<C, Parent>& ptr, const char* file, int line) noexcept
		{
			if (ptr.isNull()) return;
			void* base = ptr._basePointer();
			reline_new_impl(base, file, line);
		}
#endif
	}
}
