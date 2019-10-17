#pragma once

#include "type.h"
#include "rawdata.h"

namespace kr
{
	size_t getElementSize(JsTypedArrayType type) noexcept;

	class JsNewArray
	{
	public:
		JsNewArray(size_t size = 0) noexcept;

		const size_t size;
	};

	class JsNewArrayBuffer
	{
	public:
		JsNewArrayBuffer(size_t bytes) noexcept;
		const size_t bytes;
	};

	class JsNewTypedArray
	{
	public:
		JsNewTypedArray(JsRawData arrayBuffer, JsTypedArrayType type, size_t size = 0) noexcept;
		JsNewTypedArray(JsTypedArrayType type, size_t size = 0) noexcept;
		JsRawData arrayBuffer;
		JsTypedArrayType type;
		const size_t size;
	};
}