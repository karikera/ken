#pragma once

#include "../../main.h"
#include "../../meta/boolidx.h"
#include "../../data/iterator.h"
#include "../memlib.h"
#include "../io/stream.h"
#include "data/data.h"
#include "buffer/transbuffer.h"
#include "buffer/membuffer.h"
#include "buffer/textbuffer.h"

namespace kr
{
	template <typename C, bool copyTo, bool writeTo, bool _szable, bool _readonly, class Parent>
	class BufferInfo : public AddContainer<C, _readonly, Parent>
	{
		CLASS_HEADER(BufferInfo, AddContainer<C, _readonly, Parent>);
	public:
		INHERIT_COMPONENT();
		using Super::Super;
		static constexpr bool accessable = !copyTo && !writeTo;
		static constexpr bool copyToDefined = copyTo;
		static constexpr bool writeToDefined = writeTo;
		static constexpr bool readonly = _readonly;
		static constexpr bool szable = _szable;
	};

	namespace _pri_
	{
		template <class Derived, typename Component, bool _accessable, bool _string, bool _readonly, typename Info>
		struct Bufferable_next;
		template <class Derived, typename Component, typename Info>
		struct Bufferable_next<Derived, Component, true, false, true, Info>
		{
			using type = buffer::MemBuffer<Derived, Info>;
		};
		template <class Derived, typename Component, typename Info>
		struct Bufferable_next<Derived, Component, true, false, false, Info>
		{
			using type = buffer::WMemBuffer<Derived, Info>;
		};
		template <class Derived, typename Component, typename Info>
		struct Bufferable_next<Derived, Component, true, true, true, Info>
		{
			using type = buffer::TextBuffer<Derived, buffer::MemBuffer<Derived, Info>>;
		};
		template <class Derived, typename Component, typename Info>
		struct Bufferable_next<Derived, Component, true, true, false, Info>
		{
			using type = buffer::WTextBuffer<Derived, buffer::WMemBuffer<Derived, Info>>;
		};
		template <class Derived, typename Component, bool _string, bool _readonly, typename Info>
		struct Bufferable_next<Derived, Component, false, _string, _readonly, Info>
		{
			using type = buffer::TransBuffer<Component, Derived, Info>;
		};
		template <class Derived, bool _string, typename Info, bool _readonly>
		class Bufferable_next<Derived, AutoComponent, true, _string, _readonly, Info>;

		template <typename Derived, typename Info>
		using bufferable_next_t = typename Bufferable_next<Derived, typename Info::Component, Info::accessable, Info::is_string, Info::readonly, Info>::type;
	}

	// 버퍼러블 CRTP
	// 메모리 범위를 가지는 버퍼이다.
	// 컴포넌트 타입이 같다면, Array나 BArray 등에 넣을 수 있다.
	// 컴포넌트가 char라면, AText나 BText 등에 넣을 수 있다.
	template <class Derived, class Info>
	class Bufferable :public _pri_::bufferable_next_t<Derived, Info>
	{
		CLASS_HEADER(Bufferable, _pri_::bufferable_next_t<Derived, Info>);
	public:
		INHERIT_COMPONENT();

		using Super::Super;
	};

	class nullterm_t : public Bufferable<nullterm_t, BufferInfo<AutoComponent, true, true, true, true> >
	{
	public:
		template <class _Derived, typename _Component, class _Parent>
		inline void $writeTo(OutStream<_Derived, _Component, StreamInfo<true, _Parent>>* os) const
		{
			raw(*os->padding(1)) = (_Component)'\0';
		}
		template <typename C>
		inline size_t $copyTo(C * dest) const noexcept
		{
			*dest = (C)'\0';
			return 0;
		}

		template <typename C>
		inline size_t $sizeAs() const
		{
			return 0;
		}
	};
	
	namespace _pri_
	{
		template <typename C> 
		class SingleWrite:public Bufferable<SingleWrite<C>, BufferInfo<C, false, false, false, true>>
		{
		private:
			const C m_value;

		public:

			inline SingleWrite(C _value) noexcept
				: m_value(_value)
			{
			}

			inline size_t $size() const noexcept
			{
				return 1;
			}
			inline const internal_component_t<C>* $begin() const noexcept
			{
				return &m_value;
			}
			inline const internal_component_t<C>* $end() const noexcept
			{
				return &m_value+1;
			}
		};
		class BooleanWrite :public Bufferable<BooleanWrite, BufferInfo<AutoComponent>>
		{
		private:
			const bool m_value;

		public:
			inline BooleanWrite(bool _value) noexcept
				: m_value(_value)
			{
			}

			template <typename T>
			inline size_t $sizeAs() const noexcept
			{
				return m_value ? 4 : 5;
			}
			template <typename T>
			inline size_t $copyTo(T * dest) const noexcept
			{
				if (m_value)
				{
					*dest++ = 't';
					*dest++ = 'r';
					*dest++ = 'u';
					*dest++ = 'e';
					return 4;
				}
				else
				{
					*dest++ = 'f';
					*dest++ = 'a';
					*dest++ = 'l';
					*dest++ = 's';
					*dest++ = 'e';
					return 5;
				}
			}
		};
		class NullPointerWrite :public Bufferable<NullPointerWrite, BufferInfo<AutoComponent>>
		{
		public:
			inline NullPointerWrite(nullptr_t) noexcept
			{
			}

			template <typename T>
			inline size_t $sizeAs() const noexcept
			{
				return 7;
			}
			template <typename T>
			inline size_t $copyTo(T* dest) const noexcept
			{
				*dest++ = 'n';
				*dest++ = 'u';
				*dest++ = 'l';
				*dest++ = 'l';
				*dest++ = 'p';
				*dest++ = 't';
				*dest++ = 'r';
				return 7;
			}
		};

		template <typename array_t>
		using array_component_t = std::remove_const_t<std::remove_reference_t<decltype(**((array_t*)nullptr))>>;
		
		template <typename T, size_t idx> struct BufferizeImpl
		{
			static_assert(idx < 0, "Cannot bufferize T");
		};
		template <typename T> struct BufferizeImpl<T, 0>
		{
			using type = SingleWrite<T>;
		};
		template <typename T> struct BufferizeImpl<T, 1>
		{
			using type = decltype(numberBufferize((T)0));
		};
		template <typename T> struct BufferizeImpl<T, 2>
		{
			using type = const T&;
		};
	}
	template <typename T, typename C>
	struct Bufferize : _pri_::BufferizeImpl < T,
		meta::bool_index<
		is_same<T, C>::value,
		std::is_arithmetic<T>::value,
		IsBuffer<T>::value,
		true
		>::value
	>
	{
	};
	template <typename T, typename C> struct Bufferize<const T*, C> { using type = View<T>; };
	template <typename T, size_t count, typename C> struct Bufferize<const T[count], C> { using type = View<T>; };
	template <typename T, typename C> struct Bufferize<T*, C> { using type = View<T>; };
	template <typename C> struct Bufferize<const void*, C> { using type = NumberAddress; };
	template <typename C> struct Bufferize<void*, C> { using type = NumberAddress; };
	template <typename T, typename C, size_t sz> struct Bufferize<T[sz], C> { using type = View<T>; };
	template <typename C> struct Bufferize<nullterm_t, C> { using type = nullterm_t; };
	template <typename C> struct Bufferize<nullptr_t, C> { using type = _pri_::NullPointerWrite; };
	template <typename C> struct Bufferize<bool, C> { using type = _pri_::BooleanWrite; };
	template <typename C, typename _Traits, typename _Alloc> struct Bufferize<std::basic_string<C, _Traits, _Alloc>, C> { using type = kr::View<C>; };
}
