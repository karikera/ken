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

	// �ڹٽ�ũ��Ʈ ���� ������ ������ �ִ� Ŭ�����̴�.
	// �� �ʵ�� ���ܰ� �߻��� ��ġ�� ���� �����̴�.
	class JsException
	{
		friend _pri_::InternalTools;
	public:
		//// ���� ��
		//JsString fileName;
		//// ���� ����
		//int lineNumber;
		//// ���� ��ġ ����
		//int startColumn;
		//// ���� ��ġ ��
		//int endColumn;
		//// �ش� ������ �ҽ�
		//JsString sourceLine;
		//// �Լ� ȣ�� ����
		//JsString stackTrace;
		//// ������ ���� ����
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