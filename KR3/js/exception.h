#pragma once

#include "type.h"
#include "string.h"
#include "object.h"
#include "rawdata.h"

#ifdef KRJS_USE_V8

namespace v8
{
	class TryCatch;
}

namespace kr
{
	using JsRawException = v8::TryCatch*;
}
#else

namespace kr
{
	using JsRawException = JsValueRef;
}

#endif

namespace kr
{

	// 자바스크립트 예외 정보를 가지고 있는 클래스이다.
	// 각 필드는 예외가 발생한 위치에 대한 정보이다.
	class JsException
	{
		friend _pri_::InternalTools;
	public:
		//// 파일 명
		//JsString fileName;
		//// 파일 라인
		//int lineNumber;
		//// 가로 위치 시작
		//int startColumn;
		//// 가로 위치 끝
		//int endColumn;
		//// 해당 라인의 소스
		//JsString sourceLine;
		//// 함수 호출 스택
		//JsString stackTrace;
		//// 오류에 대한 정보
		//JsString message;

		// JsException();
		KRJS_EXPORT JsException() noexcept;
		KRJS_EXPORT JsException(Text16 message) noexcept;
		KRJS_EXPORT JsException(JsException&& _move) noexcept;

		// It will free string after out of scope
		KRJS_EXPORT Text16 toString() const noexcept;
		KRJS_EXPORT JsValue getValue() const noexcept;
		KRJS_EXPORT bool isEmpty() const noexcept;

		inline JsException& operator =(JsException&& _move) noexcept
		{
			new (this) JsException(move(_move));
			this->~JsException();
			return *this;
		}

	private:
		JsRawException m_exception;
	};

	class JsExceptionCatcher
	{
	public:
		KRJS_EXPORT JsExceptionCatcher() noexcept;
		KRJS_EXPORT ~JsExceptionCatcher() throws(kr::JsException);

	private:
#ifdef KRJS_USE_V8
		v8::TryCatch* m_trycatch;
#endif
	};
}