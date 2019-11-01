#pragma once

#include "../container.h"
#include "../../meta/types.h"

namespace kr
{
	namespace _pri_
	{
		template <typename ... ARGS>
		struct HasNullTerm;

		template <typename T, typename ... ARGS>
		struct HasNullTerm<T, ARGS...> :HasNullTerm<ARGS...>
		{
		};
		template <typename ... ARGS>
		struct HasNullTerm<nullterm_t, ARGS...>
		{
			static constexpr bool value = true;
		};
		template <>
		struct HasNullTerm<>
		{
			static constexpr bool value = false;
		};
		
		template <class Derived, typename C, class Parent>
		class OStream_cmpAccessable<Derived, C, StreamInfo<false, Parent> >
			:public AddContainer<C, true, StreamInfo<false, Parent> >
		{
			CLASS_HEADER(OStream_cmpAccessable, AddContainer<C, true, StreamInfo<false, Parent> >);
		public:
			using Super::Super;
			using typename Super::Component;
			using typename Super::InternalComponent;

			inline void write(const C * comp, size_t size) throws(NotEnoughSpaceException)
			{
				return static_cast<Derived*>(this)->$write(comp, size);
			}
			inline void writeFill(const InternalComponent & data, size_t size) throws(NotEnoughSpaceException)
			{
				TmpArray<Component> arr(size);
				arr.fill(data);
				write(arr.data(), size);
			}
		};;

		template <class Derived, typename C, class Parent>
		class OStream_cmpAccessable<Derived, C, StreamInfo<true, Parent> >
			:public AddContainer<C, false, StreamInfo<true, Parent> >
		{
			CLASS_HEADER(OStream_cmpAccessable, AddContainer<C, false, StreamInfo<true, Parent> >);
		public:
			INHERIT_COMPONENT();

		public:
			using Super::Super;
			inline InternalComponentRef* end() noexcept
			{
				return static_cast<Derived*>(this)->$end();
			}
			inline const Component* end() const noexcept
			{
				return static_cast<const Derived*>(this)->$end();
			}
			inline void _addEnd(size_t size) noexcept
			{
				return static_cast<Derived*>(this)->$_addEnd(size);
			}
			inline InternalComponentRef* limit() noexcept
			{
				return static_cast<Derived*>(this)->$limit();
			}
			inline const Component* limit() const noexcept
			{
				return static_cast<const Derived*>(this)->$limit();
			}
			inline size_t remaining() const noexcept
			{
				return static_cast<const Derived*>(this)->$remaining();
			}
			inline size_t capacity() const noexcept
			{
				return static_cast<const Derived*>(this)->$capacity();
			}

			// reserve memory with increase size
			// no constructor
			inline ComponentRef* _extend(size_t size) throws(NotEnoughSpaceException)
			{
				return static_cast<Derived*>(this)->$_extend(size);
			}
			// reserve memory without increase size
			// no constructor
			inline ComponentRef* _padding(size_t size) throws(NotEnoughSpaceException)
			{
				return static_cast<Derived*>(this)->$_padding(size);
			}

			// capacity를 늘리지 않고, 예외 없이 강제로 준비한다.
			inline Component* prepareForce(size_t size) noexcept
			{
				Component * e = end();
				_addEnd(size);
				mema::ctor(e, size);
				return e;
			}
			inline Component* prepare(size_t size) throws(NotEnoughSpaceException)
			{
				Component * e = _extend(size);
				mema::ctor(e, size);
				return e;
			}
			inline void writeForce(const InternalComponent & data) noexcept
			{
				*end() = data;
				_addEnd(1);
			}
			inline void writeForce(const Component* data, size_t size) noexcept
			{
				Component * e = end();
				_addEnd(size);
				mema::ctor_copy((InternalComponent*)e, (const InternalComponent*)data, size);
			}
			inline void write(const Component* data, size_t size) throws(NotEnoughSpaceException)
			{
				Component * e = _extend(size);
				mema::ctor_copy((InternalComponent*)e, (const InternalComponent*)data, size);
			}
			inline void writeFill(const InternalComponent & data, size_t size) throws(NotEnoughSpaceException)
			{
				Component * e = _extend(size);
				mema::ctor_fill(e, data, size);
			}
			template <typename T>
			inline void writeas(const T &value) throws(NotEnoughSpaceException)
			{
				static_assert(sizeof(T) % sizeof(InternalComponent) == 0, "Size of T must aligned by size of component");
				return write((Component*)&value, sizeof(T) / sizeof(InternalComponent));
			}
			// without constructor
			inline Component * padding(size_t size) throws(NotEnoughSpaceException)
			{
				if (size == 0) return end();
				return _padding(size);
			}
			// without constructor
			inline void commit(size_t sz)
			{
				if (sz == 0) return;
				_addEnd(sz);
			}
			template <typename ... ARGS>
			void prints(const ARGS & ... args) throws(NotEnoughSpaceException)
			{
				constexpr bool hasNullTerm = HasNullTerm<ARGS...>::value;
				meta::types<bufferize_t<ARGS, Component> ...> datas = { ((bufferize_t<ARGS, Component>)args) ... };
				size_t size = 0;
				datas.value_loop([&size](auto & data) {
					size += data.template sizeAs<Component>();
				});
				if (hasNullTerm) size++;
				padding(size);
				datas.value_loop([this](auto & data) { data.writeTo(static_cast<Derived*>(this)); });
			}
		};
		
		/// C2504: Info must be StreamInfo class
		template <class Derived, typename C, typename Info>
		class OStream_cmpComponent :public OStream_cmpAccessable<Derived, C, Info>
		{
			CLASS_HEADER(OStream_cmpComponent, OStream_cmpAccessable<Derived, C, Info>);
		public:
			INHERIT_COMPONENT();
			using Super::Super;
		};

		template <class Derived, class Info>
		class OStream_cmpComponent<Derived, AutoComponent, Info>;

		template <typename OS, typename IS, bool os_buffered, bool is_buffered, size_t BSIZE>
		class Pipe
		{
		public:
			static constexpr size_t BUFFER_SIZE = BSIZE != -1 ? BSIZE : 8192;
			static_assert(is_same<typename IS::Component, typename OS::Component>::value, "Is not same type");
			using Component = typename OS::Component;
			using InternalComponent = typename OS::InternalComponent;

		private:
			IS* m_is;
			OS* m_os;
			TmpArray<byte> m_buffer;
			InternalComponent* m_writep;
			InternalComponent* m_readp;

		public:

			Pipe(OS * os, IS * is) noexcept
				:m_buffer(BUFFER_SIZE)
			{
				m_is = is;
				m_os = os;
				m_readp = m_writep = (InternalComponent*)m_buffer.data();
			}

			void streaming()
			{
				if (m_writep == m_readp)
				{
					m_readp = m_writep = (InternalComponent*)m_buffer.data();
					size_t readed = m_is->read(m_writep, BUFFER_SIZE / sizeof(InternalComponent));
					m_writep += readed;
				}
				m_os->write(m_readp, m_writep - m_readp);
				m_readp = m_writep;
			}
		};

		template <typename OS, typename IS, bool os_buffered>
		class Pipe<OS, IS, os_buffered, true, (size_t)-1>
		{
		public:
			static_assert(is_same<typename IS::Component, typename OS::Component>::value, "Is not same type");
			using Component = typename OS::Component;

			Pipe(OS * os, IS * is) noexcept
			{
				m_is = is;
				m_os = os;
			}
			bool streaming()
			{
				size_t size = 8192;
				const Component* src = m_is->read(&size);
				m_os->write(src, size);
			}
		private:
			IS* m_is;
			OS* m_os;
		};

		template <typename OS, typename IS>
		class Pipe<OS, IS, true, false, (size_t)-1>
		{
		public:
			static_assert(is_same<typename IS::Component, typename OS::Component>::value, "Is not same type");
			using Component = typename OS::Component;
			static constexpr size_t MINIMUM_BUFFER = 2048;

			Pipe(OS * os, IS * is) noexcept
			{
				m_is = is;
				m_os = os;
			}
			bool streaming()
			{
				if (m_os->remaining() < MINIMUM_BUFFER) m_os->padding(MINIMUM_BUFFER);
				size_t sz = m_is->read(m_os->end(), m_os->remaining());
				m_os->commit(sz);
				return true;
			}
		private:
			IS* m_is;
			OS* m_os;
		};
	}

	template <typename OS, typename IS, size_t BSIZE = (size_t)-1> class Pipe
		: public _pri_::Pipe<OS, IS, OS::accessable, IS::accessable, BSIZE>
	{
		static_assert(IsIStream<IS>::value, "IS is not InStream");
		static_assert(IsOStream<OS>::value, "OS is not OutStream");
		CLASS_HEADER(Pipe, _pri_::Pipe<OS, IS, OS::accessable, IS::accessable, BSIZE>);
	public:
		using Super::Super;
	};

	template <class Derived, typename Component, typename Info>
	class OutStream :public _pri_::OStream_cmpComponent<Derived, Component, Info>
	{
		CLASS_HEADER(OutStream, _pri_::OStream_cmpComponent<Derived, Component, Info>);

	public:
		INHERIT_COMPONENT();

		using Super::Super;
		using Super::write;

		void write(const InternalComponent & data) throws(NotEnoughSpaceException)
		{
			write((const Component*)&data, 1);
		}
		void write(Ref data) throws(NotEnoughSpaceException)
		{
			write(data.begin(), data.size());
		}
		void writeLeb128(dword value) throws(NotEnoughSpaceException)
		{
			InternalComponent result;
			for (;;)
			{
				result = (InternalComponent)(value & 0x7f);
				value >>= 7;
				if (value == 0)
					break;
				result |= 0x80;
				write((Component*)&result, 1);
			}
			write((Component*)&result, 1);
		}
		void writeLeb128(qword value) throws(NotEnoughSpaceException)
		{
			InternalComponent result;
			for (;;)
			{
				result = (InternalComponent)(value & 0x7f);
				value >>= 7;
				if (value == 0)
					break;
				result |= 0x80;
				write((Component*)&result, 1);
			}
			write((Component*)&result, 1);
		}

		template <typename _Derived, typename _Info>
		void passThrough(InStream<_Derived, Component, _Info> * is)
		{
			try
			{
				Pipe<OutStream, InStream<_Derived, Component, _Info>> pipe(this, is);
				for (;;) pipe.streaming();
			}
			catch (EofException&)
			{
			}
		}

		template <typename T>
		void print(const T & v) throws(...)
		{
			using buffer_ref_t = bufferize_t<T, Component>;
			using buffer_t = remove_constref_t<buffer_ref_t>;
			using buffer_component_t = typename buffer_t::Component;
			static_assert(is_same<buffer_component_t, AutoComponent>::value || is_same<buffer_component_t, Component>::value, "Unmatch component type"); // Unmatch component type
			buffer_ref_t(v).writeTo(this);
		}
		template <typename _Wrapped, bool _sized>
		void print(const Writable<Component, _Wrapped, _sized>& v) throws(...)
		{
			v.writeTo(this);
		}
		template <typename T>
		void writeas(const T & data) throws(NotEnoughSpaceException)
		{
			static_assert(sizeof(T) % sizeof(InternalComponent) == 0, "component size unmatch");
			write((const Component*)&data, sizeof(T) / sizeof(InternalComponent));
		}

		template <typename T>
		OutStream& operator << (const T& arrayable) throws(NotEnoughSpaceException)
		{
			print(arrayable);
			return *this;
		}
		OutStream& operator << (OutStream& (*fn)(OutStream&))
		{
			_assert(fn != nullptr);
			return ((*fn)(*this));
		}
	};
	
	template <typename C, typename OS>
	class CastedAutoOutStream:public OutStream<CastedAutoOutStream<C, OS>, C, StreamInfo<false> >
	{
	public:
		void $write(const C* data, size_t size) noexcept
		{
			return reinterpret_cast<OS*>(this)->write(data, size);
		}
	};

	template <class Derived, class Parent>
	class OutStream<Derived, AutoComponent, StreamInfo<false, Parent>>
		:public AddContainer<AutoComponent, true, StreamInfo<false, Parent>>
	{
		CLASS_HEADER(OutStream, AddContainer<AutoComponent, true, StreamInfo<false, Parent>>);
	public:
		INHERIT_COMPONENT();
		using Super::Super;

		template <typename C>
		inline void write(const C* comp, size_t size) throws(NotEnoughSpaceException)
		{
			return static_cast<Derived*>(this)->$write(comp, size);
		}
		template <typename C>
		inline void writeFill(const internal_component_t<C>& data, size_t size) throws(NotEnoughSpaceException)
		{
			TmpArray<internal_component_t<C>> arr(size);
			arr.fill(data);
			write(arr.data(), size);
		}
		template <typename C>
		void write(const C& data) throws(NotEnoughSpaceException)
		{
			write(&data, 1);
		}
		template <typename C>
		void write(View<C> data) throws(NotEnoughSpaceException)
		{
			write(data.begin(), data.size());
		}

		template <typename _Derived, typename _Component, typename _Info>
		void passThrough(InStream<_Derived, _Component, _Info>* is)
		{
			try
			{
				constexpr bool vIsAuto = is_same<_Component, AutoComponent>::value;
				CastedAutoOutStream<void, This>* stream = cast<meta::if_t<vIsAuto, void, _Component>>();
				Pipe<CastedAutoOutStream<void, This>, InStream<_Derived, _Component, _Info>> pipe(stream, is);
				for (;;) pipe.streaming();
			}
			catch (EofException&)
			{
			}
		}

		template <typename C>
		CastedAutoOutStream<C, This>* cast() noexcept
		{
			return reinterpret_cast<CastedAutoOutStream<C, This>*>(this);
		}

		template <typename T>
		void print(const T& v) throws(NotEnoughSpaceException)
		{
			using buffer_ref_t = bufferize_t<T, Component>;
			using buffer_t = remove_constref_t<buffer_ref_t>;
			using buffer_component_t = typename buffer_t::Component;
			constexpr bool vIsAuto = is_same<buffer_component_t, AutoComponent>::value;
			buffer_t(v).writeTo(cast<meta::if_t<vIsAuto, void, buffer_component_t>>());
		}
		template <typename T>
		void writeas(const T& data) throws(NotEnoughSpaceException)
		{
			write(data);
		}

		template <typename T>
		OutStream& operator << (const T& arrayable) throws(NotEnoughSpaceException)
		{
			print(arrayable);
			return *this;
		}
		OutStream& operator << (OutStream& (*fn)(OutStream&))
		{
			_assert(fn != nullptr);
			return ((*fn)(*this));
		}
	};

	namespace io
	{
		template <typename C>
		class SizeOStream :public OutStream<SizeOStream<C>, C>
		{
		private:
			size_t m_size;

		public:
			SizeOStream() noexcept
			{
				m_size = 0;
			}

			size_t size() const noexcept
			{
				return m_size;
			}
			SizeOStream & operator += (size_t _sz) noexcept
			{
				m_size += _sz;
				return *this;
			}
			void $write(const C *, size_t _sz) noexcept
			{
				m_size += _sz;
			}
		};
	}

}
