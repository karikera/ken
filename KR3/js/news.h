#pragma once

#include "type.h"
#include "rawdata.h"

namespace kr
{
	size_t getElementSize(JsTypedType type) noexcept;

	class JsNewSymbol
	{
	public:
		JsNewSymbol() noexcept;
		JsNewSymbol(JsRawData desc) noexcept;
		const JsRawData description;
	};

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
		JsNewTypedArray(JsRawData arrayBuffer, JsTypedType type, size_t size = 0) noexcept;
		JsNewTypedArray(JsTypedType type, size_t size = 0) noexcept;
		JsRawData arrayBuffer;
		JsTypedType type;
		const size_t size;
	};
}