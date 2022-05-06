#pragma once

#include "../../main.h"
#include "../../meta/boolidx.h"
#include "../memlib.h"
#include "../io/stream.h"
#include "hasmethod.h"
#include "data/data.h"
#include "buffer/method.h"
#include "buffer/membuffer.h"
#include "buffer/textbuffer.h"

namespace kr
{
	template <typename C, template <typename, typename> class Method, bool _szable, bool _readonly, class Parent>
	class BufferInfo : public AddContainer<C, _readonly, Parent>
	{
		CLASS_HEADER(AddContainer<C, _readonly, Parent>);
	public:
		INHERIT_COMPONENT();
		using Super::Super;
		static constexpr bool szable = _szable;
		static constexpr bool readonly = _readonly;
	};

	class nullterm_t final : public Bufferable<nullterm_t, BufferInfo<AutoComponent, method::CopyWriteTo, true, true> >
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
	static constexpr const nullterm_t nullterm = nullterm_t();

	namespace _pri_
	{
		template <typename C>
		class SingleWrite :public HasStreamTo<SingleWrite<C>, C, Bufferable<SingleWrite<C>, BufferInfo<C, method::Memory, false, true> > >
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
				return &m_value + 1;
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
				using Name = Names<T>;
				if (m_value)
				{
					memcpy(dest, Name::_true, sizeof(Name::_true));
					return countof(Name::_true);
				}
				else
				{
					memcpy(dest, Name::_false, sizeof(Name::_false));
					return countof(Name::_false);
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
				using Name = Names<T>;
				memcpy(dest, Name::_nullptr, sizeof(Name::_nullptr));
				return countof(Name::_nullptr);
			}
		};
		template <typename T, bool isNumber>
		struct BufferizeSwitch
		{
			using type = decltype(_pri_::numberBufferize(declval<T>()));
		};
		template <typename T>
		struct BufferizeSwitch<T, false>
		{
			using type = const T&;
		};
	}

	template <typename T, typename C>
	struct Bufferize:_pri_::BufferizeSwitch<T, std::is_arithmetic<T>::value>
	{
	};
	template <typename C> struct Bufferize<C, C> { using type = _pri_::SingleWrite<C>; };
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

extern template struct kr::Names<char>;
extern template struct kr::Names<kr::char16>;
