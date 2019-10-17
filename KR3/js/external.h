#pragma once

#include <KR3/main.h>
#include "type.h"
#include "rawdata.h"

namespace kr
{
	class JsExternal :public Interface<>
	{
	public:
		KRJS_EXPORT JsExternal() noexcept;
		KRJS_EXPORT ~JsExternal() noexcept;

		virtual void remove() noexcept = 0;

		void AddRef() noexcept;
		void Release() noexcept;

	protected:
		int m_reference;
		JsRawExternal m_external;

		KRJS_EXPORT void _release() noexcept;
	};
}
