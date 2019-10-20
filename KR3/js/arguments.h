#pragma once

#include "type.h"
#include "function.h"
#include "object.h"

namespace kr
{

	// 자바스크립트 함수의 파라미터 목록
	class JsArguments
	{
	private:
		JsValue m_callee;
		JsValue m_this;
		Array<JsValue> m_arguments;

	public:
		JsArguments(const JsValue&_callee, const JsValue&_this, JsArgumentsIn args) noexcept;
		JsArguments(const JsValue&_callee, const JsValue&_this, size_t sz) noexcept;
		JsArguments(JsArguments&& _move) noexcept;
		~JsArguments() noexcept;

		JsArguments& operator =(JsArguments&& _move) noexcept;
		JsValue& operator [](size_t i) noexcept;
		const JsValue& operator [](size_t i) const noexcept;
		size_t size() const noexcept;

		const JsValue& getCallee() const noexcept;
		const JsValue& getThis() const noexcept;
		JsValue* begin() noexcept;
		JsValue* end() noexcept;
		const JsValue* begin() const noexcept;
		const JsValue* end() const noexcept;
	};

}