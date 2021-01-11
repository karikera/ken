#pragma once

#include <KR3/main.h>
#include <KR3/util/initpack.h>
#include "type.h"

namespace kr
{
	class JsRuntime
	{
	public:
		using Init = JsRuntime;
		static InitPack initpack;

		KRJS_EXPORT JsRuntime() noexcept;
		KRJS_EXPORT ~JsRuntime() noexcept;
		KRJS_EXPORT static JsValue global() noexcept;
		KRJS_EXPORT static JsValue run(Text16 fileName, Text16 source) throws(JsException);
		KRJS_EXPORT static JsValue run(Text16 fileName, Text16 source, uintptr_t sourceContext) throws(JsException);
		static JsValue run(Text16 source) throws(JsException);

		KRJS_EXPORT static const JsRawRuntime& getRaw() noexcept;
		KRJS_EXPORT static void setRuntime(const JsRawRuntime& runtime) noexcept;
		KRJS_EXPORT static void dispose() noexcept;
		KRJS_EXPORT static void gc() noexcept;
		KRJS_EXPORT static void idle() noexcept;
		KRJS_EXPORT static void test() noexcept;
	};

	class JsScope
	{
		friend _pri_::InternalTools;
	public:
		KRJS_EXPORT JsScope() noexcept;
		KRJS_EXPORT ~JsScope() noexcept;

	private:
#ifdef KRJS_USE_V8
#else
#endif
	};

	// ���� V8 ������ ����� �� �ִ� Ŭ�����̴�.
	// execute �� ȣ���Ͽ� �����Ѵ�.
	class JsContext
	{
		friend _pri_::InternalTools;
		friend _pri_::JsClassInfo;
		friend JsRawData;
		friend JsClass;
		friend JsRuntime;
	public:
		class Scope
		{
		public:
			Scope(JsContext& ctx) noexcept;
			~Scope() noexcept;

		private:
			JsContext* const m_context;
		};
		KRJS_EXPORT JsContext() noexcept;
		KRJS_EXPORT JsContext(const JsContext& _ctx) noexcept;
		KRJS_EXPORT JsContext(const JsRawContext &ctx) noexcept;
		KRJS_EXPORT ~JsContext() noexcept;

		KRJS_EXPORT const JsRawContext& getRaw() noexcept;
		KRJS_EXPORT void enter() noexcept;
		KRJS_EXPORT void exit() noexcept;
		KRJS_EXPORT static void exitCurrent() noexcept;
		KRJS_EXPORT static void _exit() noexcept;
		KRJS_EXPORT static void _cleanStackCounter() noexcept;

	private:
		JsRawContext m_context;
		JsRawDataValue m_global;
		JsRawDataValue m_undefinedValue;
		JsRawDataValue m_trueValue;
		JsRawDataValue m_falseValue;
		JsRawDataValue m_nullValue;
		JsRawPropertyId m_prototypeId;
		JsRawPropertyId m_constructorId;
		JsRawPropertyId m_lengthId;
		JsRawPropertyId m_getId;
		JsRawPropertyId m_setId;
		Array<JsRawData> m_classes;

	public:

		inline const JsRawContext& getRaw() const noexcept
		{
			return m_context;
		}
	};
}
