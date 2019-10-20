#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <new>

#include "ken-v8/vken.h"
#include "ken-v8/ref.h"
#ifdef __KR3_INCLUDED
#include "ken-v8/meta.h"
#endif
#include "ken-v8/exports.h"
#include "ken-v8/external.h"
#include "ken-v8/jsvar.h"
#include "ken-v8/v8data.h"
#include "ken-v8/private.h"
#include "ken-v8/class.h"
#include "ken-v8/nativefunction.h"
#include "ken-v8/nativeclass.h"
#include "ken-v8/exception.h"
#include "ken-v8/arguments.h"

#ifdef __KR3_INCLUDED
#include "ken-v8/meta.inl"
#endif
#include "ken-v8/jsvar.inl"
#include "ken-v8/private.inl"
#include "ken-v8/class.inl"
#include "ken-v8/v8data.inl"

namespace kr
{
	// 컴파일된 자바스크립트
	class JsCode
	{
		friend JsContext;
	public:
		// 자바스크립트를 컴파일 시킨다.
		// 파라미터 fileName: 예외가 발생하면, 발생 파일 이름에 넣어놓을 문자열
		// 파라미터 source: 소스 문자열
		KR_EASYV8_DLLEXPORT JsCode() noexcept;
		KR_EASYV8_DLLEXPORT JsCode(JsText fileName, JsText source); // JsException
		KR_EASYV8_DLLEXPORT ~JsCode() noexcept;
		KR_EASYV8_DLLEXPORT JsCode(const JsCode &_copy) noexcept;
		KR_EASYV8_DLLEXPORT bool empty() const noexcept;
		inline JsCode& operator =(const JsCode &_copy) noexcept
		{
			this->~JsCode();
			new (this) JsCode(_copy);
			return *this;
		}
#ifdef __KR3_INCLUDED
		JsCode(Text16 fileName, Text16 source)
			:JsCode(fileName.data(), fileName.size(), source.data(), source.size())
		{
		}
#endif

	private:
		v8::Persistent<v8::Script> m_script;
	};

	// 쉽게 V8 엔진을 사용할 수 있는 클래스이다.
	// execute 를 호출하여 실행한다.
	class JsContext
	{
		friend JsCode;
		friend V8Object;
	public:
		KR_EASYV8_DLLEXPORT JsContext() noexcept;
		KR_EASYV8_DLLEXPORT JsContext(const JsContext & _ctx) noexcept;
		KR_EASYV8_DLLEXPORT ~JsContext() noexcept;
		
		// 자바스크립트를 실행한다.
		// 예외: JsException
		KR_EASYV8_DLLEXPORT JsAny run(const JsCode & code);
		KR_EASYV8_DLLEXPORT V8Object global();
		KR_EASYV8_DLLEXPORT void enter();
		KR_EASYV8_DLLEXPORT void exit();

		inline JsCode compile(JsText source) // JsException
		{
			return JsCode(u"[untitled]", source);
		}
		inline JsCode compile(JsText fileName, JsText source) // JsException
		{
			return JsCode(fileName, source);
		}
		inline JsAny run(JsText fileName, JsText source) // JsException
		{
			return run(compile(fileName, source));
		}
		inline JsAny run(JsText source) // JsException
		{
			return run(compile(source));
		}

		KR_EASYV8_DLLEXPORT static void gc() noexcept;
		KR_EASYV8_DLLEXPORT static void terminate() noexcept;

#ifdef __KR3_INCLUDED
		inline JsCode compile(Text16 source) // JsException
		{
			return compile({source.data(), source.size());
		}
		inline JsCode compile(Text16 fileName, Text16 source) // JsException
		{
			return compile({ fileName.data(), fileName.size() }, { source.data(), source.size() });
		}
		inline JsAny run(Text16 fileName, Text16 source) // JsException
		{
			return run(compile({ fileName.data(), fileName.size() }, { source.data(), source.size() }));
		}
		inline JsAny run(Text16 source) // JsException
		{
			return run(compile({ source.data(), source.size() }));
		}
#endif
		
	private:
		v8::Persistent<v8::Context> m_context;
	};
	class JsContextScope
	{
	public:
		inline JsContextScope(const JsContext & ctx) noexcept;
		inline ~JsContextScope() noexcept;

	private:
		JsContext m_ctx;
	};
	inline JsContextScope::JsContextScope(const JsContext & ctx) noexcept
		:m_ctx(ctx)
	{
		m_ctx.enter();
	}
	inline JsContextScope::~JsContextScope() noexcept
	{
		m_ctx.exit();
	}
}
