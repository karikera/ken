#pragma once
#include "private.h"

template <typename T>
inline kr::JsField<T>::JsField(int index) noexcept
	:m_index(index)
{
}
template <typename T>
int kr::JsField<T>::getIndex() const noexcept
{
	return m_index;
}

inline kr::JsFieldMaker::JsFieldMaker(kr::V8Class * cls) noexcept
	: m_class(cls), m_count(cls->getInternalFieldCount())
{
}
inline kr::JsFieldMaker::~JsFieldMaker() noexcept
{
	m_class->setInternalFieldCount(m_count);
}
template <typename T>
inline void kr::JsFieldMaker::make(JsField<T> * field) noexcept
{
	*field = JsField<T>(m_count++);
}
#ifdef __KR3_INCLUDED
template <typename T>
inline void kr::JsFieldMaker::make(Text16 name, JsField<T> * field) noexcept
{
	make(field);
	m_class->makeField(name, JsFilter(field->getIndex(), [](const JsAny & val)->JsAny { return val.cast<T>(); }));
}
template <typename T>
inline void kr::JsFieldMaker::makeReadOnly(Text16 name, JsField<T> * field) noexcept
{
	make(field);
	m_class->makeReadOnlyField(name, field->getIndex());
}
#endif