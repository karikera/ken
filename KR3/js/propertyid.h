#pragma once

#include "type.h"

namespace kr
{
	class JsPropertyId
	{
		friend JsRawData;

	public:
		KRJS_EXPORT explicit JsPropertyId(pcstr16 name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(const JsRawPropertyId& name) noexcept;
		KRJS_EXPORT explicit JsPropertyId(JsRawPropertyId&& name) noexcept;
		KRJS_EXPORT ~JsPropertyId() noexcept;


	private:
		JsRawPropertyId m_data;
	};
}