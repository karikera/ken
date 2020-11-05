#pragma once

namespace kr
{
	namespace ary
	{
		// begin,end 등이 Parent에 정의되어있으므로, AddBufferable에 Parent를 직접 연결한다
		template <class Parent> class BufferIMethod : 
			public AddBufferable<Parent, BufferInfo<typename Parent::Component, method::Memory, false, !Parent::writable, Parent>>
		{
			CLASS_HEADER(BufferIMethod, AddBufferable<Parent, BufferInfo<typename Parent::Component, method::Memory, false, !Parent::writable, Parent>>);
		public:
			INHERIT_ARRAY();

			using Super::Super;
			using Super::size;
			using Super::begin;
			using Super::end;
			using Super::empty;

			Ref operator +(intptr_t n) const noexcept
			{
				return Ref(begin() + n, end());
			}
			Ref operator -(intptr_t n) const noexcept
			{
				return Ref(begin() - n, end());
			}
			intptr_t operator -(const Component* ptr) const noexcept
			{
				return begin() - (InternalComponent*)ptr;
			}
			friend intptr_t operator -(const Component* ptr, const BufferIMethod& ori) noexcept
			{
				return (InternalComponent*)ptr - (InternalComponent*)ori.begin();
			}
			template <typename _Parent> intptr_t operator -(const BufferIMethod<_Parent>& ptr) const noexcept
			{
				return begin() - (internal_component_t<typename _Parent::Component>*)ptr.begin();
			}
		};
		template <class Parent> class BufferIConstructor :
			public Parent
		{
			CLASS_HEADER(BufferIConstructor, Parent);
		public:
			INHERIT_ARRAY();

			using Super::Super;

			constexpr BufferIConstructor()  noexcept = default;
			constexpr BufferIConstructor(const BufferIConstructor&) = default;
			constexpr BufferIConstructor(BufferIConstructor&&) = default;

			template <class _Derived, bool a, bool b, class _Parent>
			BufferIConstructor(buffer::Memory<_Derived, BufferInfo<Component, method::Memory, a, b, _Parent>>& data) noexcept
				:Super(data.begin(), data.end())
			{
			}
			template <class _Derived, bool a, bool b, class _Parent>
			BufferIConstructor(const buffer::Memory<_Derived, BufferInfo<Component, method::Memory, a, b, _Parent>>& data) noexcept
				:Super(data.begin(), data.end())
			{
			}
			template <typename _Traits, typename _Alloc>
			BufferIConstructor(std::basic_string<Component, _Traits, _Alloc>& str) noexcept
				: Super(str.data(), str.size())
			{
			}
			template <typename _Traits, typename _Alloc>
			BufferIConstructor(const std::basic_string<Component, _Traits, _Alloc>& str) noexcept
				:Super(str.data(), str.size())
			{
			}
		};
		template <class Parent> class BufferIVoidConstructor :
			public Parent
		{
			CLASS_HEADER(BufferIVoidConstructor, Parent);
		public:
			INHERIT_ARRAY();

			using Super::Super;

			constexpr BufferIVoidConstructor()  noexcept = default;
			constexpr BufferIVoidConstructor(const BufferIVoidConstructor&) = default;
			constexpr BufferIVoidConstructor(BufferIVoidConstructor&&) = default;

			template <class _Derived, typename C, bool a, bool b, class _Parent>
			BufferIVoidConstructor(buffer::Memory<_Derived, BufferInfo<C, method::Memory, a, b, _Parent>>& data) noexcept
				:Super(data.begin(), data.end())
			{
			}
			template <class _Derived, typename C, bool a, bool b, class _Parent>
			BufferIVoidConstructor(const buffer::Memory<_Derived, BufferInfo<C, method::Memory, a, b, _Parent>>& data) noexcept
				:Super(data.begin(), data.end())
			{
			}
			template <typename _Traits, typename C, typename _Alloc>
			BufferIVoidConstructor(std::basic_string<C, _Traits, _Alloc>& str) noexcept
				: Super(str.data(), str.size())
			{
			}
			template <typename _Traits, typename C, typename _Alloc>
			BufferIVoidConstructor(const std::basic_string<C, _Traits, _Alloc>& str) noexcept
				:Super(str.data(), str.size())
			{
			}
		};
	}
}
