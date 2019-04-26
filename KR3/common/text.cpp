#include "stdafx.h"
#include "text.h"

#ifdef _MSC_VER
template class kr::ary::_pri_::WrapImpl<kr::ary::data::AccessableData<char, kr::Empty>, char>;
template class kr::ary::_pri_::WrapImpl<kr::ary::data::ReadableData<char, kr::Empty>, char>;
template class kr::ary::_pri_::WrapImpl<kr::ary::data::AllocatedData<char, kr::Empty>, char>;
//template class kr::ary::_pri_::WrapImpl<kr::ary::data::TemporaryData<char, kr::Empty>, char>;
//template class kr::ary::_pri_::WrapImpl<kr::ary::data::WritableData<char, kr::Empty>, char>;

template class kr::ary::_pri_::WrapImpl<kr::ary::data::AccessableData<char16, kr::Empty>, char16>;
template class kr::ary::_pri_::WrapImpl<kr::ary::data::ReadableData<char16, kr::Empty>, char16>;
template class kr::ary::_pri_::WrapImpl<kr::ary::data::AllocatedData<char16, kr::Empty>, char16>;
//template class kr::ary::_pri_::WrapImpl<kr::ary::data::TemporaryData<char16, kr::Empty>, char16>;
//template class kr::ary::_pri_::WrapImpl<kr::ary::data::WritableData<char16, kr::Empty>, char16>;
#endif
