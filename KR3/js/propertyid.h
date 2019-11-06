#pragma once

#include "type.h"

namespace kr
{
	class JsPropertyId
	{
		friend JsRawData;

	public:
		KRJS_EXPORT JsPropertyId() noexcept;
		KRJS_EXPORT explicit JsPropertyId(pcstr16 name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(const JsPropertyId& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(JsPropertyId&& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(const JsRawPropertyId& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(JsRawPropertyId&& name) noexcept;
		KRJS_EXPORT ~JsPropertyId() noexcept;

		JsPropertyId& operator =(const JsPropertyId& name) noexcept;
		JsPropertyId& operator =(JsPropertyId&& name) noexcept;

	private:
		JsRawPropertyId m_data;
	};
}