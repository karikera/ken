#pragma once

#include "../container.h"

namespace kr
{
	namespace _pri_
	{
		template <typename T>
		class PointableContainer
		{
		private:
			T m_data;

		public:
			using Component = typename T::Component;

			PointableContainer(T&& data) noexcept
				:m_data(move(data))
			{
			}
			PointableContainer(const T& data) noexcept
				:m_data(data)
			{
			}
			const T* operator ->() const noexcept
			{
				return &m_data;
			}
			T* operator ->() noexcept
			{
				return &m_data;
			}
		};
		template <typename T>
		class PointableContainer<T*>
		{
		private:
			T* m_data;

		public:
			using Component = typename T::Component;

			PointableContainer(T* data) noexcept
				:m_data(data)
			{
			}
			T* operator ->() const noexcept
			{
				return m_data;
			}
		};
	}
	template <typename C, typename Wrapped, bool sized = false>
	class Writable
	{
		using Container = _pri_::PointableContainer<Wrapped>;
		static_assert(is_same<C, typename Container::Component>::value, "component type unmatch");
	protected:
		mutable Container m_wrapped;

	public:

		Writable(Wrapped && wrapped) noexcept
			:m_wrapped(move(wrapped))
		{
		}
		Writable(const Wrapped& wrapped) noexcept
			:m_wrapped(wrapped)
		{
		}

		template <class _Derived, class _Parent>
		void writeTo(OutStream<_Derived, C, StreamInfo<true, _Parent> >* os) const throws(...)
		{
			try
			{
				for (;;)
				{
					size_t sz = m_wrapped->$read(os->padding(4096), 4096);
					os->commit(sz);
				}
			}
			catch (EofException&)
			{
			}
		}
	};

	template <typename C, typename Wrapped>
	class Writable<C, Wrapped, true>:public Writable<C, Wrapped, false>
	{
		using Super = Writable<C, Wrapped, false>;
	protected:
		using Super::m_wrapped;
		mutable size_t m_size;

	public:

		Writable(const Wrapped &wrapped, size_t size) noexcept
			:Writable<C, Wrapped, false>(wrapped), m_size(size)
		{
		}
		Writable(Wrapped&& wrapped, size_t size) noexcept
			:Writable<C, Wrapped, false>(move(wrapped)), m_size(size)
		{
		}

		size_t size() const noexcept
		{
			return m_size;
		}
		template <class _Derived, class _Parent>
		void writeTo(OutStream<_Derived, C, StreamInfo<true, _Parent>>* os) const noexcept
		{
			size_t sz = m_wrapped->read(os->padding(m_size), m_size);
			m_size -= sz;
			os->commit(sz);
		}
	};

	namespace _pri_
	{
		template <class Derived, typename C, class Info>
		class IStream_cmpAccessable<Derived, C, StreamInfo<false, Info>>
			: public AddContainer<C, true, StreamInfo<false, Info>>
		{
			CLASS_HEADER(IStream_cmpAccessable, AddContainer<C, true, StreamInfo<false, Info>>);
		public:
			INHERIT_COMPONENT();

			using Super::Super;
			using TSZ = TempSzText<C>;
			
			inline size_t read(Component * dest, size_t sz) throws(...)
			{
				return static_cast<Derived*>(this)->$read(dest, sz);
			}
			inline InternalComponent read() throws(...)
			{
				InternalComponent out;
				read((Component*)&out, 1);
				return out;
			}
			inline size_t $skip(size_t size) throws(...)
			{
				TmpArray<InternalComponent> tempbuffer(size);
				return static_cast<Derived*>(this)->$read(tempbuffer.data(), size);
			}
			inline size_t skip(size_t sz) throws(...)
			{
				return static_cast<Derived*>(this)->$skip(sz);
			}
			inline size_t skipAll() throws(...)
			{
				size_t skipped = 0;
				try
				{
					for (;;)
					{
						skipped += static_cast<Derived*>(this)->$skip(4096);
					}
				}
				catch (EofException&)
				{
				}
				return skipped;
			}
			
			template <typename T> T readas() throws(...)
			{
				T value;
				size_t dwLen = read(&value, sizeof(T));
				if (dwLen != sizeof(T)) throw EofException();
				return value;
			}
		};

		template <class Derived, typename C, class Info>
		class IStream_cmpAccessable<Derived, C, StreamInfo<true, Info>> :
			public StreamInfo<true, Info>
		{
			CLASS_HEADER(IStream_cmpAccessable, StreamInfo<true, Info>);
		public:
			INHERIT_COMPONENT();
			using Super::Super;
			
			inline const InternalComponent* begin() const noexcept
			{
				return static_cast<const Derived*>(this)->$begin();
			}
			inline const InternalComponent* end() const noexcept
			{
				return static_cast<const Derived*>(this)->$end();
			}
			inline size_t size() const noexcept
			{
				return static_cast<const Derived*>(this)->$size();
			}
			inline bool empty() const noexcept
			{
				return static_cast<const Derived*>(this)->emptyImpl();
			}

		private:
			inline Derived* derived() noexcept
			{
				return static_cast<Derived*>(this);
			}

		public:
			using Buffer = AddBufferable<Super, BufferInfo<C, false, false, false, true, Super> >;
			inline Buffer* buffer() noexcept
			{
				return static_cast<Buffer*>(static_cast<Super*>(this));
			}

			inline ComponentRef* read(size_t * psize) throws(EofException)
			{
				return derived()->$read(psize);
			}
			inline size_t skip(size_t sz) throws(EofException)
			{
				read(&sz);
				return sz;
			}
			inline size_t skipAll() noexcept
			{
				size_t sz = size();
				derived()->setBegin(end());
				return sz;
			}
			inline size_t read(Component * dest, size_t sz) throws(EofException)
			{
				const Component* src = read(&sz);
				mema::subs_copy((InternalComponent*)dest, (InternalComponent*)src, sz);
				return sz;
			}

			// 예외 검사 없이, 강제로 읽는다, 
			inline InternalComponent readForce() noexcept
			{
				const InternalComponent * p = begin();
				derived()->addBegin(1);
				return *p;
			}
			inline InternalComponent read() throws(EofException)
			{
				if (size() == 0)
					throw EofException();
				const InternalComponent * p = begin();
				derived()->addBegin(1);
				return *p;
			}
			inline Ref read(size_t _len) throws(EofException)
			{
				if (size() == 0) throw EofException();
				_len = mint(_len, size());
				Ref out(begin(), _len);
				derived()->addBegin(_len);
				return out;
			}
			template <typename T>
			inline T readas() throws(EofException)
			{
				static_assert(sizeof(T) % sizeof(InternalComponent) == 0, "Size of T must aligned by size of component");
				Ref ref = read(sizeof(T) / sizeof(InternalComponent));
				return *(T*)ref.begin();
			}
			inline Ref _readto_p(const Component* p) noexcept
			{
				const InternalComponent* b = begin();
				const InternalComponent* e = end();

				_assert(b <= p && p <= e);
				Ref out(b, p);
				derived()->setBegin((InternalComponent*)p);
				return out;
			}
			inline Ref _readto_p(const Component* p, size_t skip) noexcept
			{
				const InternalComponent* b = begin();
				const InternalComponent* e = end();
				_assert(b <= p && (InternalComponent*)p + skip <= e);
				Ref out(b, p);
				derived()->setBegin((InternalComponent*)p + skip);
				return out;
			}
			inline Ref readto_p(const Component* _idx) noexcept
			{
				return _idx == nullptr ? (Ref)nullptr : _readto_p(_idx);
			}
			inline Ref readto_p(const Component* _idx, size_t _skip) noexcept
			{
				return _idx == nullptr ? (Ref)nullptr : _readto_p(_idx, _skip);
			}
			inline Ref readto_pe(const Component* _idx) noexcept
			{
				return _idx == nullptr ? (Ref)readAll() : _readto_p(_idx);
			}
			inline Ref readto_pe(const Component* _idx, size_t _skip) noexcept
			{
				return _idx == nullptr ? (Ref)readAll() : _readto_p(_idx, _skip);
			}

			inline Ref readto(const InternalComponent&_needle) noexcept
			{
				return readto_p(buffer()->find(_needle));
			}
			inline Ref readto(Ref _needle) noexcept
			{
				return readto_p(buffer()->find(_needle));
			}
			inline Ref readto(Ref _needle, size_t _skip) noexcept
			{
				return readto_p(buffer()->find(_needle), _skip);
			}
			inline Ref readto_e(const InternalComponent& _needle) noexcept
			{
				return readto_pe(buffer()->find(_needle));
			}
			inline Ref readto_e(Ref _needle) noexcept
			{
				return readto_pe(buffer()->find(_needle));
			}
			inline Ref readto_e(Ref _needle, size_t _skip) noexcept
			{
				return readto_pe(buffer()->find(_needle), _skip);
			}
			inline Ref readto_y(Ref _cut) noexcept
			{
				return readto_p(buffer()->find_y(_cut));
			}
			inline Ref readto_y(Ref _cut, size_t _skip) noexcept
			{
				return readto_p(buffer()->find_y(_cut), _skip);
			}
			inline Ref readto_ye(Ref _cut) noexcept
			{
				return readto_pe(buffer()->find_y(_cut));
			}
			inline Ref readto_ye(Ref _cut, size_t _skip) noexcept
			{
				return readto_pe(buffer()->find_y(_cut), _skip);
			}
			inline Ref readto_n(const InternalComponent &_cut) noexcept
			{
				return readto_pe(buffer()->find_n(_cut));
			}
			inline Ref readto_n(const InternalComponent &_cut, size_t _skip) noexcept
			{
				return readto_pe(buffer()->find_n(_cut), _skip);
			}
			inline Ref readto_ny(Ref _cut) noexcept
			{
				return readto_pe(buffer()->find_ny(_cut));
			}
			inline Ref readto_ny(Ref _cut, size_t _skip) noexcept
			{
				return readto_pe(buffer()->find_ny(_cut), _skip);
			}
			template <typename LAMBDA>
			inline Ref readto_L(const LAMBDA &lambda)
			{
				return readto_pe(buffer()->find_L(lambda));
			}
			template <typename LAMBDA>
			inline Ref readto_eL(const LAMBDA &lambda)
			{
				return readto_pe(buffer()->find_L(lambda));
			}
			inline Ref skipspace()
			{
				return readto_ny(Ref::WHITE_SPACE);
			}
			inline Ref readwith(const InternalComponent &_cut) noexcept
			{
				return readto_p(buffer()->find(_cut), 1);
			}
			inline Ref readwith_e(const InternalComponent &_cut) noexcept
			{
				return readto_pe(buffer()->find(_cut), 1);
			}
			inline Ref readwith(Ref _cut) noexcept
			{
				return readto(_cut, _cut.size());
			}
			inline Ref readwith_e(Ref _cut) noexcept
			{
				return readto_e(_cut, _cut.size());
			}
			inline Ref readwith_y(Ref _cut) noexcept
			{
				return readto_y(_cut, 1);
			}
			inline Ref readwith_ye(Ref _cut) noexcept
			{
				return readto_ye(_cut, 1);
			}
			inline Ref readwith_n(const InternalComponent &_cut) noexcept
			{
				return readto_n(_cut, 1);
			}
			inline Ref readwith_ny(Ref _cut) noexcept
			{
				return readto_ny(_cut, 1);
			}
			template <typename LAMBDA>
			inline Ref readwith_L(const LAMBDA & lambda)
			{
				return readto_p(buffer()->find_L(lambda), 1);
			}
			template <typename LAMBDA>
			inline Ref readwith_eL(const LAMBDA & lambda)
			{
				return readto_pe(buffer()->find_L(lambda), 1);
			}
			template <class _Derived, class _Parent>
			inline void readAll(OutStream<_Derived, Component, StreamInfo<true, _Parent>> * os) noexcept
			{
				static_cast<Derived*>(this)->read(os, size());
			}
			inline Ref readAll() noexcept
			{
				Ref out(begin(), end());
				derived()->setBegin(end());
				return out;
			}
			inline size_t readwith_a(Alc *_v, const InternalComponent &_cut) noexcept
			{
				return _v->alloc(readwith(_cut));
			}

			inline size_t readwith_ay(Alc *_v, Ref _cut) noexcept
			{
				return _v->alloc(readwith(_cut));
			}
			inline size_t read_str(Wri& _v) noexcept
			{
				size_t _len = _v.copy(*this);
				derived()->addBegin(_len);
				return _len;
			}
			inline size_t readwith_str(Wri& _v, const InternalComponent &_cut) noexcept
			{
				return _v.copy(readwith(_cut));
			}
			inline size_t readwith_str_y(Wri& _v, Ref _cut) noexcept
			{
				return _v.copy(readwith(_cut));
			}

			inline int read_enumchar(Ref list) throws(EofException)
			{
				KR_DEFINE_MMEM();
				if (size() == 0)
					throw EofException();
				size_t i = memm::pos(list.begin(), *begin(), list.size());
				if (i != -1)
					derived()->addBegin(1);
				return i;
			}
		};

		template <class Derived, typename C, class Info>
		class IStream_voidStream: public IStream_cmpAccessable<Derived, C, Info>
		{
			CLASS_HEADER(IStream_voidStream, IStream_cmpAccessable<Derived, C, Info>);
		public:
			using Super::Super;
		};

		template <class Derived, class Info>
		class IStream_voidStream<Derived, void, Info>: public IStream_cmpAccessable<Derived, void, Info>
		{
			CLASS_HEADER(IStream_voidStream, IStream_cmpAccessable<Derived, void, Info>);
		public:
			INHERIT_COMPONENT();
			using Super::Super;
			using Super::read;
			using Super::skip;
			using Super::readas;

			bool testSignature(dword signature) noexcept
			{
				dword dwSignature;
				uintptr_t upLen = read(&dwSignature, 4);
				if (upLen != 4) return false;
				if (dwSignature != signature) return false;
				return true;
			}
			dword findChunk(dword signature) throws(EofException)
			{
				dword dwSize;
				while (!testSignature(signature))
				{
					dwSize = this->template readas<dword>();
					skip(dwSize);
				}
				dwSize = this->template readas<dword>();
				return dwSize;
			}
			void readStructure(ptr value, uintptr_t size) throws(EofException)
			{
				_assert(this != nullptr);
				*(dword*)value = this->template readas<dword>();
				readStructure((dword*)value + 1, size - sizeof(dword), *(dword*)value - sizeof(dword));
			}
			void readStructure(ptr value, uintptr_t size, uintptr_t srcsize) throws(EofException)
			{
				byte* pRead = (byte*)value;
				if (srcsize < size)
				{
					srcsize = read(pRead, srcsize);
					mem::zero(pRead + srcsize, size - srcsize);
				}
				else
				{
					uintptr_t readed = read(pRead, size);
					if (readed < size) mem::zero(pRead + size, size - readed);
					if (readed < srcsize) skip(srcsize - readed);
				}
			}
		};
	}

	template <class Derived, typename Component, typename Info>
	class InStream
		: public _pri_::IStream_voidStream<Derived, Component, Info>
	{
		CLASS_HEADER(InStream, _pri_::IStream_voidStream<Derived, Component, Info>);
	public:
		INHERIT_COMPONENT();
		using Super::Super;
		using Super::read;

		inline Writable<Component, Derived*> readAll() throws(...)
		{
			return static_cast<Derived*>(this);
		}
		inline Writable<Component, Derived*, true> read(size_t size) throws(...)
		{
			return { static_cast<Derived*>(this), size };
		}

		inline dword readLeb128() throws(...)
		{
			dword result = 0;
			dword shift = 0;
			while (true)
			{
				InternalComponent v;
				read(&v, 1);
				result |= (v & 0x7f) << shift;
				if ((v & 0x80) == 0)
					break;
				shift += 7;
			}
			return result;
		}

		inline qword readLeb128_64() throws(...)
		{
			qword result = 0;
			dword shift = 0;
			while (true)
			{
				Component v;
				read(&v, 1);
				result |= (qword)(v & 0x7f) << shift;
				if ((v & 0x80) == 0)
					break;
				shift += 7;
			}
			return result;
		}

		inline dword readLeb128_kr() throws(...)
		{
			dword v = 0;
			byte shift = 0;
			for (;;)
			{
				Component d;
				read(&d, 1);
				if ((d & 0x80) != 0)
				{
					v |= (d & 0x7f) << shift;
					break;
				}
				v |= d << shift;
				shift += 7;
			}
			return v;
		}

		inline qword readLeb128_kr64() throws(...)
		{
			qword v = 0;
			byte shift = 0;
			for (;;)
			{
				Component d;
				read(&d, 1);
				if ((d & 0x80) != 0)
				{
					v |= (qword)(d & 0x7f) << shift;
					break;
				}
				v |= (qword)(d << shift);
				shift += 7;
			}
			return v;
		}
	};
}
