#pragma once

#include "container.h"
#include "io/lock.h"
#include "io/stream.h"
#include "text/iterator.h"
#include "text/buffer.h"
#include "text/method/buffer_i.h"
#include "text/method/buffer_io.h"
#include "text/method/stream.h"
#include "text/method/utils.h"
#include "text/wrap.h"
#include "text/numformat.h"
#include "text/nullterm.h"
#include "text/join.h"
#include "text/encoder.h"

namespace kr
{
	template <typename C>
	class TempSzText:public TmpArray<C>
	{
		CLASS_HEADER(TempSzText, TmpArray<C>);
	public:
		INHERIT_COMPONENT();

		using Super::c_str;
		using Super::prints;

		TempSzText() noexcept
			:Super()
		{
		}
		TempSzText(size_t size) noexcept
			: Super(size)
		{
		}
		TempSzText(size_t size, size_t capacity) noexcept
			: Super(size, capacity)
		{
		}
		TempSzText(TempSzText&& _move) noexcept
			:Super((Super&&)_move)
		{
		}
		TempSzText(Super&& _move) noexcept
			: Super(move(_move))
		{
		}

		TempSzText& operator = (const TempSzText &) noexcept = delete;

		template <typename T> TempSzText& operator <<(const T& temp)
		{
			*(Super*)this << temp;
			return *this;
		}
		operator C*() noexcept
		{
			return c_str();
		}
	};

	inline kr::Text operator ""_tx(const char * tx, size_t sz)
	{
		return kr::Text(tx, sz);
	}
	inline kr::Text16 operator ""_tx(const char16 * tx, size_t sz)
	{
		return kr::Text16(tx, sz);
	}
	inline kr::Text32 operator ""_tx(const char32 * tx, size_t sz)
	{
		return kr::Text32(tx, sz);
	}
}

template <typename C>
struct std::hash<kr::View<C>>
{
	size_t operator ()(const kr::View<C> & buffer) const noexcept
	{
		return kr::mem::hash(buffer.begin(), buffer.sizeBytes());
	}
};
template <typename C>
struct std::hash<kr::Array<C>>
{
	size_t operator ()(const kr::View<C> & buffer) const noexcept
	{
		return kr::mem::hash(buffer.begin(), buffer.sizeBytes());
	}
};
template <typename C, size_t cap>
struct std::hash<kr::BArray<C, cap>>
{
	size_t operator ()(const kr::BArray<C, cap> & buffer) const noexcept
	{
		return kr::mem::hash(buffer.begin(), buffer.sizeBytes());
	}
};

#ifdef _MSC_VER
extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::AccessableData<char, kr::Empty>, char>;
extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::ReadableData<char, kr::Empty>, char>;
extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::AllocatedData<char, kr::Empty>, char>;
//extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::TemporaryData<char, kr::Empty>, char>;
//extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::WritableData<char, kr::Empty>, char>;

extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::AccessableData<char16, kr::Empty>, char16>;
extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::ReadableData<char16, kr::Empty>, char16>;
extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::AllocatedData<char16, kr::Empty>, char16>;
//extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::TemporaryData<char16, kr::Empty>, char16>;
//extern template class kr::ary::_pri_::WrapImpl<kr::ary::data::WritableData<char16, kr::Empty>, char16>;
#endif
