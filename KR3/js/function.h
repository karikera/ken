#pragma once

#include "type.h"
#include "meta.h"
#include "external.h"
#include "value.h"

namespace kr
{
	// 네이티브 함수
	// 자바스크립트로 네이티브 함수를 넣기 위해 사용
	class JsFunction
	{
		friend _pri_::InternalTools;
	private:
		class Data:public Referencable<Data>
		{
		public:
			KRJS_EXPORT JsValue create() noexcept;
			virtual JsValue call(const JsArguments & args) = 0;
		};

		template <typename LAMBDA> class LambdaWrap : public Data
		{
		private:
			const LAMBDA m_lambda;

		public:
			LambdaWrap(LAMBDA lambda) noexcept
				:m_lambda(lambda)
			{
			}
			virtual JsValue call(const JsArguments & args) override
			{
				return m_lambda(args);
			}

		};

		Data * m_data;
	public:

		JsFunction() noexcept;
		bool isEmpty() const noexcept;
		~JsFunction() noexcept;
		JsFunction(const JsFunction& func) noexcept;
		JsFunction(JsFunction&& func) noexcept;
		JsValue create() const noexcept;
		JsValue call(const JsArguments& args) const noexcept;
		JsFunction& operator =(const JsFunction& _copy) noexcept;
		JsFunction& operator =(JsFunction&& _copy) noexcept;
		bool operator ==(const JsFunction& o) const noexcept;
		bool operator !=(const JsFunction& o) const noexcept;

		template <typename FUNC>
		friend class JsFunctionT;

		template <typename LAMBDA>
		JsFunction(LAMBDA func) noexcept
		{
			m_data = _new LambdaWrap<LAMBDA>(func);
			m_data->AddRef();
		}
		template <typename FUNC>
		JsFunction(const JsFunctionT<FUNC>& func) noexcept
		{
			m_data = func.m_data;
			m_data->AddRef();
		}
		template <typename FUNC>
		JsFunction(JsFunctionT<FUNC>&& func) noexcept
		{
			m_data = func.m_data;
			func.m_data = nullptr;
		}
		template <typename FUNC>
		JsFunction& operator =(const JsFunctionT<FUNC>& _copy) noexcept
		{
			this->~JsFunction();
			new(this) JsFunction(_copy);
			return *this;
		}
		template <typename FUNC>
		JsFunction& operator =(JsFunctionT<FUNC>&& _copy) noexcept
		{
			this->~JsFunction();
			new(this) JsFunction(move(_copy));
			return *this;
		}


		template <typename LAMBDA>
		static JsValue make(LAMBDA func) noexcept;

		template <typename LAMBDA>
		static JsValue makeT(LAMBDA func) noexcept;
	};

	template <typename RET, typename ... ARGS> 
	class JsFunctionT<RET(ARGS ...)> :public JsFunction
	{
	public:

		// 일반 함수를 자바스크립트 함수로 만들어냅니다.
		// 제한된 타입만 사용할 수 있습니다.
		// 사용 가능한 타입: bool, int, float, double, std::wstring
		template <typename LAMBDA> JsFunctionT(LAMBDA func) noexcept
		{
			auto lambda = [func](const JsArguments & args)->JsValue
			{
				return JsMeta<LAMBDA>::Call::call(func, args);
			};
			m_data = _new LambdaWrap<decltype(lambda)>(move(lambda));
			m_data->AddRef();
		}

		// 객체의 함수를 호출합니다.
		RET call(JsObject _this, const ARGS & ... args) const
		{
			JsArguments jsargs(_this, sizeof ... (args));

			size_t i = sizeof ... (args)-1;
			unpackR((jsargs[i--] = args));

			return JsFunction::call(jsargs).get<RET>();
		}
	};
}

template <typename LAMBDA>
kr::JsValue kr::JsFunction::make(LAMBDA func) noexcept
{
	return JsFunction(func).create();
}
template <typename LAMBDA>
kr::JsValue kr::JsFunction::makeT(LAMBDA func) noexcept
{
	return typename JsMeta<LAMBDA>::ccfunc(func).create();
}