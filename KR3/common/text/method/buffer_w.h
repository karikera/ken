#pragma once

namespace kr
{
	namespace ary
	{
		template <class Parent> 
		class BufferWMethod: public Parent
		{
			CLASS_HEADER(Parent);
		public:
			INHERIT_ARRAY();

			using Super::begin;
			using Super::end;
			using Super::Super;

			constexpr BufferWMethod()  noexcept = default;
			constexpr BufferWMethod(const BufferWMethod&) = default;
			constexpr BufferWMethod(BufferWMethod&&) = default;

			Wri toWriter() noexcept
			{
				return Wri(begin(), end());
			}
		};
	}
}