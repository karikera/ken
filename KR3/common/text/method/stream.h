#pragma once

#include "../../io/istream.h"

namespace kr
{
	namespace ary
	{
		template <class Parent>
		class IStreamMethod: public InStream<IStreamMethod<Parent>, typename Parent::Component, StreamInfo<true, Parent>>
		{
			CLASS_HEADER(IStreamMethod, InStream<IStreamMethod<Parent>, typename Parent::Component, StreamInfo<true, Parent>>);
		public:
			INHERIT_ARRAY();

			using Super::Super;

			using Super::addBegin;
			using Super::setBegin;

			using Super::begin;
			using Super::end;
			using Super::size;
			using Super::empty;

			Self& operator ++() noexcept
			{
				addBegin(1);
				return *static_cast<Self*>(this);
			}
			Ref operator ++(int) noexcept
			{
				Ref out = *static_cast<Self*>(this);
				addBegin(1);
				return out;
			}
			Self& operator --() noexcept
			{
				addBegin(-1);
				return *static_cast<Self*>(this);
			}
			Ref operator --(int) noexcept
			{
				Ref out = *static_cast<Self*>(this);
				addBegin(-1);
				return out;
			}
			Self& operator +=(intptr_t value) noexcept
			{
				addBegin(value);
				return *static_cast<Self*>(this);
			}
			Self& operator -=(intptr_t value) noexcept
			{
				addBegin(-value);
				return *static_cast<Self*>(this);
			}

			inline void subarr_self(size_t _size) noexcept
			{
				_assert(_size <= size());
				addBegin(_size);
			}
			inline void subarr_self(const Component* _ptr) noexcept
			{
				_assert(_ptr == nullptr || begin() <= _ptr && _ptr <= end());
				setBegin(_ptr);
			}

			constexpr IStreamMethod() = default;
			constexpr IStreamMethod(const IStreamMethod&) = default;
			constexpr IStreamMethod(IStreamMethod&&) = default;
			template <class _Derived, class _Parent>
			constexpr IStreamMethod(const InStream<_Derived, Component, StreamInfo<true, _Parent>> & data) noexcept
				:Super(data.begin(), data.end())
			{
			}
		};
		template <class Parent>
		class OStreamMethod 
			: public OutStream<OStreamMethod<Parent>, typename Parent::Component, StreamInfo<true, Parent>>
		{
			CLASS_HEADER(OStreamMethod, OutStream<OStreamMethod<Parent>, typename Parent::Component, StreamInfo<true, Parent>>);
		public:
			INHERIT_ARRAY();

			using Super::Super;
			using Super::end;
			using Super::limit;

			constexpr OStreamMethod() = default;
			constexpr OStreamMethod(const OStreamMethod&) = default;
			constexpr OStreamMethod(OStreamMethod&&) = default;
			template <class _Derived, class _Parent>
			constexpr OStreamMethod(const OutStream<_Derived, Component, StreamInfo<true, _Parent>> & data) noexcept
				:Super(data.end(), data.limit())
			{
			}
		};
		template <class Parent>
		class IRStreamMethod
			: public Parent
		{
			CLASS_HEADER(IRStreamMethod, Parent);
		public:
			INHERIT_ARRAY();

			using Super::Super;
			using Super::begin;
			using Super::end;
			using Super::size;
			using Super::setEnd;
			using Super::addEnd;
			using Super::empty;

			void skipBack() throws(EofException)
			{
				if (empty())
					throw EofException();
				setEnd(end() - 1);
			}
			void skipBack(size_t count) throws(EofException)
			{
				if (size() < count) throw EofException();
				setEnd(end()-count);
			}
			InternalComponent readBack() throws(EofException)
			{
				if (empty()) throw EofException();
				InternalComponentRef* endptr = end() - 1;
				setEnd(endptr);
				return *endptr;
			}
			Ref readBack(size_t count) throws(EofException)
			{
				if (size() < count) throw EofException();
				InternalComponentRef* endptr = end();
				InternalComponentRef* comp = endptr - count;
				setEnd(comp);
				return Ref(comp, endptr);
			}
			void cut_self(InternalComponentRef* newend) noexcept
			{
				_assert(begin() <= newend && newend <= end());
				setEnd(newend);
			}
			void cut_self(size_t _len) noexcept
			{
				return cut_self(mint(begin() + _len, end()));
			}
			InternalComponent popGet() throws(EofException)
			{
				return readBack();
			}
			void pop() throws(EofException)
			{
				if (empty()) throw EofException();
				addEnd(-1);
			}

			template <typename T>
			T &readBackAs() throws(EofException)
			{
				static_assert(sizeof(T) % sizeof(InternalComponent) == 0, "Size of T must aligned by size of component");
				Ref ref = readBack(sizeof(T));
				return *(T*)ref.data();
			}
		};
	}
}