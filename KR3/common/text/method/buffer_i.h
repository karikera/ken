#pragma once

namespace kr
{
	namespace ary
	{
		namespace method
		{
			// begin,end 등이 Parent에 정의되어있으므로, AddBufferable에 Parent를 직접 연결한다
			template <class Parent> class BufferIMethod : 
				public AddBufferable<Parent, BufferInfo<typename Parent::Component, false, false, false, !Parent::writable, Parent>>
			{
				CLASS_HEADER(BufferIMethod, AddBufferable<Parent, BufferInfo<typename Parent::Component, false, false, false, !Parent::writable, Parent>>);
			public:
				INHERIT_ARRAY();

				using Super::Super;
				using Super::size;
				using Super::begin;
				using Super::end;
				using Super::empty;

				constexpr BufferIMethod() noexcept = default;
				constexpr BufferIMethod(const BufferIMethod&) = default;
				constexpr BufferIMethod(BufferIMethod&&) = default;

				template <class _Derived, bool a, bool b, class _Parent>
				BufferIMethod(Bufferable<_Derived, BufferInfo<Component, false, false, a, b, _Parent>> & data) noexcept
					:Super(data.begin(), data.end())
				{
				}
				template <class _Derived, bool a, bool b, class _Parent>
				BufferIMethod(const Bufferable<_Derived, BufferInfo<Component, false, false, a, b, _Parent>> & data) noexcept
					:Super(data.begin(), data.end())
				{
				}
				template <typename _Traits, typename _Alloc>
				BufferIMethod(std::basic_string<Component, _Traits, _Alloc> & str) noexcept
					: Super(str.data(), str.size())
				{
				}
				template <typename _Traits, typename _Alloc>
				BufferIMethod(const std::basic_string<Component, _Traits, _Alloc> & str) noexcept
					:Super(str.data(), str.size())
				{
				}

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
		}
	}
}
