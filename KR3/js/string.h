#pragma once

#include "type.h"
#include "value.h"

namespace kr
{
	// ½ºÆ®¸µ
	class JsString :public JsValue
	{
	public:
		JsString() noexcept;
		JsString(nullptr_t) noexcept;

		JsString(const JsString&) = default;
		JsString(JsString&&) = default;
		~JsString() = default;
		
		JsString(pcstr16 str, size_t length) noexcept;
		JsString(Text str, Charset cs = Charset::Default) noexcept;
		JsString(Text16 str) noexcept;
		JsString(int n) noexcept;
		JsString(double n) noexcept;
		JsString(bool b) noexcept;
		JsString(undefined_t) noexcept;
		JsString(void*) noexcept;
		JsString(const JsValue& value) noexcept;

		JsString& operator =(const JsString& _copy) noexcept;
		JsString& operator =(JsString&& _move) noexcept;

		Text16 get() noexcept;
		operator Text16() noexcept;
	};
}
