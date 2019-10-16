#pragma once

#include "type.h"

namespace kr
{
	class JsPersistent
	{
		friend JsRawData;
	public:
		KRJS_EXPORT JsPersistent() noexcept;
		KRJS_EXPORT JsPersistent(const JsRawData &value) noexcept;
		KRJS_EXPORT JsPersistent(const JsValue& value) noexcept;
		KRJS_EXPORT JsPersistent(const JsPersistent& obj) noexcept;
		KRJS_EXPORT JsPersistent(JsPersistent&& obj) noexcept;
		KRJS_EXPORT ~JsPersistent() noexcept;

		KRJS_EXPORT bool isEmpty() const noexcept;
		JsPersistent& operator =(const JsRawData& value) noexcept;
		JsPersistent& operator =(const JsValue& value) noexcept;
		JsPersistent& operator =(const JsPersistent& obj) noexcept;
		JsPersistent& operator =(JsPersistent&& obj) noexcept;
		
	private:
		JsRawPersistent m_data;
	};

}