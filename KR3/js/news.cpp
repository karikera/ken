#include "stdafx.h"
#include "news.h"

using namespace kr;

size_t kr::getElementSize(JsTypedArrayType type) noexcept
{
	static const size_t SIZES[] = {
		1,1,1,
		2,2,
		4,4,
		4,
		8
	};
	_assert((int)type >= 0 && (int)type < countof(SIZES));
	return SIZES[(int)type];
}

JsNewArray::JsNewArray(size_t size) noexcept
	:size(size)
{
}
JsNewArrayBuffer::JsNewArrayBuffer(size_t bytes) noexcept
	:bytes(bytes)
{
}
JsNewTypedArray::JsNewTypedArray(JsRawData arrayBuffer, JsTypedArrayType type, size_t size) noexcept
	:arrayBuffer(arrayBuffer), type(type), size(size)
{
}
JsNewTypedArray::JsNewTypedArray(JsTypedArrayType type, size_t size) noexcept
	:arrayBuffer(JsNewArrayBuffer(getElementSize(type) * size)), type(type), size(size)
{
}
