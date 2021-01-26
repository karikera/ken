#pragma once

#include "type.h"
#include "function.h"
#include "object.h"

namespace kr
{

	// 자바스크립트 함수의 파라미터 목록
	class JsArguments
	{
		friend _pri_::InternalTools;
	public:
		JsArguments(JsArguments&& _move) noexcept;
		~JsArguments() noexcept;

		JsArguments& operator =(JsArguments&& _move) noexcept;

		JsValue& operator [](size_t i) noexcept;
		const JsValue& operator [](size_t i) const noexcept;
		size_t size() const noexcept;

		const JsValue& getThis() const noexcept;

		void set(size_t idx, const JsValue& value) noexcept;

		template <typename T>
		T at(size_t i) const noexcept
		{
			if (i >= m_argn) return _pri_::JsCast::defaultValue<T>();
			return m_args[i].cast<T>();
		}
		template <typename T>
		bool equalsAt(size_t i, T&& value) const noexcept
		{
			if (i < m_argn) return is_same_v<undefined_t, decay_t<T>>;
			return m_args[i] == value;
		}

	private:
		JsValue m_this;
		const size_t m_argn;

	protected:
		JsValue* const m_args;

		JsArguments(const JsValue& _this, JsValue* args, size_t argn) noexcept;

	};

	class JsArgumentsAllocated:public JsArguments
	{
	public:
		KRJS_EXPORT JsArgumentsAllocated(const JsValue& _this, size_t argn) noexcept;
		KRJS_EXPORT ~JsArgumentsAllocated() noexcept;
	};
}