#pragma once

#include "type.h"

namespace kr
{
	class JsPropertyId
	{
		friend JsRawData;
		friend JsClass;

	public:
		KRJS_EXPORT JsPropertyId() noexcept;
		KRJS_EXPORT JsPropertyId(pcstr16 name) noexcept;
		JsPropertyId(nullptr_t)=delete;
		KRJS_EXPORT explicit JsPropertyId(const JsPropertyId& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(JsPropertyId&& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(const JsRawPropertyId& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(JsRawPropertyId&& name) noexcept;
		KRJS_EXPORT ~JsPropertyId() noexcept;

		JsPropertyId& operator =(const JsPropertyId& name) noexcept;
		JsPropertyId& operator =(JsPropertyId&& name) noexcept;

		KRJS_EXPORT static JsPropertyId fromSymbol(JsRawData value) noexcept;
	private:
		JsRawPropertyId m_data;
	};
}