#pragma once

#include <KR3/main.h>
#include "type.h"
#include "rawdata.h"

namespace kr
{
	class JsExternalData :public Interface<>
	{
	public:
		KRJS_EXPORT JsExternalData() noexcept;
		KRJS_EXPORT ~JsExternalData() noexcept;

		virtual void remove() noexcept = 0;

		void AddRef() noexcept;
		void Release() noexcept;

	protected:
		int m_reference;
		JsRawExternal m_external;

		KRJS_EXPORT void _release() noexcept;
	};
}
