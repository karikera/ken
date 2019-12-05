#pragma once

#include "../container.h"
#include "../hasmethod.h"
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
				mema::ctor(e, end());
				return e;
			}
			inline Component* prepare(size_t size) throws(NotEnoughSpaceException)
			{
				Component * e = _extend(size);
				mema::ctor(e, end());
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
				meta::types<bufferize_t<ARGS, Component>...> datas = { args ... };

				size_t size = 0;
				datas.value_loop([&](const auto & data) {
					size += data.sizeAs<Component>();
				});
				if (hasNullTerm) size++;
				InternalComponent * dest = (InternalComponent*)padding(size);
				datas.value_loop([&](const auto & data) {
					size_t sz = data.copyTo(dest);
					dest += sz;
					commit(sz);
				});
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

		struct PrintToMixed
		{
			template <class Derived, typename Component, typename Info, typename Derived2, typename Parent2>
			static void printTo(OutStream<Derived, Component, Info>* out, const HasStreamTo<Derived2, Component, Parent2>& value) throws(...)
			{
				const_cast<HasStreamTo<Derived2, Component, Parent2>&>(value).streamTo(out);
			}
			template <class Derived, typename Component, typename Info, typename Derived2, typename Parent2>
			static void printTo(OutStream<Derived, Component, Info>* out, const HasOnlyCopyTo<Derived2, Component, Parent2>& value) throws(...)
			{
				using OS = OutStream<Derived, Component, Info>;
				WriteLock<OS, value.maximum> lock;
				Component* dest = lock.lock(out);
				size_t sz = value.copyTo(dest);
				lock.unlock(out, sz);
			}
			template <class Derived, typename Component, typename Info, typename Derived2, typename Parent2>
			static void printTo(OutStream<Derived, AutoComponent, Info>* out, const HasStreamTo<Derived2, Component, Parent2>& value) throws(...)
			{
				const_cast<HasStreamTo<Derived2, Component, Parent2>&>(value).streamTo(out);
			}
			template <class Derived, typename Component, typename Info, typename Derived2, typename Parent2>
			static void printTo(OutStream<Derived, AutoComponent, Info>* out, const HasOnlyCopyTo<Derived2, Component, Parent2>& value) throws(...)
			{
				using OS = OutStream<Derived, Component, Info>;
				WriteLock<OS, value.maximum> lock;
				Component* dest = lock.lock(out);
				size_t sz = value.copyTo(dest);
				lock.unlock(out, sz);
			}
		};

		struct PrintToAsBuffer
		{
			template <class Derived, typename Component, typename Info, typename Any>
			static void printTo(OutStream<Derived, Component, Info>* out, const Any& value) throws(...)
			{
				using buffer_t = decay_t<bufferize_t<Any, Component> >;
				using buffer_component_t = typename buffer_t::Component;
				static_assert(is_same<buffer_component_t, AutoComponent>::value || is_same<buffer_component_t, Component>::value, "Unmatch component type"); // Unmatch component type
				bufferize_t<Any, Component>(value).writeTo(out);
			}
		};

		template <typename T>
		struct PrintTo:public meta::if_t<
			has_method<T, HasOnlyCopyTo>::value ||
			has_method<T, HasStreamTo>::value, 
			PrintToMixed, PrintToAsBuffer>
		{
		};
	}

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

		template <typename T>
		void print(const T& v) throws(...)
		{
			_pri_::PrintTo<remove_const_t<T> >::printTo(this, v);
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

		template <typename C>
		CastedAutoOutStream<C, This>* cast() noexcept
		{
			return reinterpret_cast<CastedAutoOutStream<C, This>*>(this);
		}

		template <typename T>
		void print(const T& v) throws(...)
		{
			_pri_::PrintTo<remove_const_t<T> >::printTo(this, v);
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

extern template class kr::io::SizeOStream<void>;
extern template class kr::io::SizeOStream<char>;
extern template class kr::io::SizeOStream<kr::char16>;
