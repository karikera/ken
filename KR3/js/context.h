#pragma once

#include <KR3/main.h>
#include "type.h"

namespace kr
{
	class JsRuntime
	{
	public:
		using Init = JsRuntime;

		KRJS_EXPORT JsRuntime() noexcept;
		KRJS_EXPORT ~JsRuntime() noexcept;
		KRJS_EXPORT static JsValue global() noexcept;
		KRJS_EXPORT static JsValue run(Text16 fileName, Text16 source) throws(JsException);
		static JsValue run(Text16 source) throws(JsException);

		KRJS_EXPORT static void setRuntime(const JsRawRuntime& runtime) noexcept;
		KRJS_EXPORT static void gc() noexcept;
	};

	// ���� V8 ������ ����� �� �ִ� Ŭ�����̴�.
	// execute �� ȣ���Ͽ� �����Ѵ�.
	class JsContext
	{
	public:
		KRJS_EXPORT JsContext() noexcept;
		KRJS_EXPORT JsContext(const JsContext& _ctx) noexcept;
		KRJS_EXPORT JsContext(const JsRawContext &ctx) noexcept;
		KRJS_EXPORT ~JsContext() noexcept;

		KRJS_EXPORT void enter() noexcept;
		KRJS_EXPORT void exit() noexcept;

	private:
		JsRawContext m_context;
		JsRawDataValue m_global;
		JsRawDataValue m_undefinedValue;
		JsRawDataValue m_trueValue;
		JsRawDataValue m_falseValue;
		JsRawDataValue m_nullValue;
		JsRawPropertyId m_prototypeId;
		JsRawPropertyId m_constructorId;
		Array<JsRawDataValue> m_classes;
	};
}
