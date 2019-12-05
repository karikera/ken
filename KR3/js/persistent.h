#pragma once

#include "type.h"

namespace kr
{
	class JsPersistent
	{
		friend JsRawData;
		friend JsWeak;
		friend _pri_::JsCast;
		template <class Class, class Parent>
		friend class JsObjectT;
	public:
		KRJS_EXPORT JsPersistent() noexcept;
		KRJS_EXPORT JsPersistent(const JsRawData &value) noexcept;
		KRJS_EXPORT JsPersistent(const JsWeak& obj) noexcept;
		KRJS_EXPORT JsPersistent(const JsPersistent& obj) noexcept;
		KRJS_EXPORT JsPersistent(JsPersistent&& obj) noexcept;
		KRJS_EXPORT ~JsPersistent() noexcept;

		KRJS_EXPORT bool isEmpty() const noexcept;
		JsPersistent& operator =(const JsRawData& value) noexcept;
		JsPersistent& operator =(const JsPersistent& obj) noexcept;
		JsPersistent& operator =(JsPersistent&& obj) noexcept;
		JsPersistent(nullptr_t) noexcept;
		JsPersistent& operator =(nullptr_t) noexcept;
		
	private:
		JsRawPersistent m_data;
	};

	class JsWeak
	{
		friend JsRawData;
		friend JsPersistent;
		friend _pri_::JsCast;
		template <class Class, class Parent>
		friend class JsObjectT;
	public:
		KRJS_EXPORT JsWeak() noexcept;
		KRJS_EXPORT JsWeak(const JsRawData& value) noexcept;
		KRJS_EXPORT JsWeak(const JsPersistent& obj) noexcept;
		KRJS_EXPORT JsWeak(const JsWeak& obj) noexcept;
		KRJS_EXPORT JsWeak(JsWeak&& obj) noexcept;
		KRJS_EXPORT ~JsWeak() noexcept;

		KRJS_EXPORT bool isEmpty() const noexcept;
		JsWeak& operator =(const JsRawData& value) noexcept;
		JsWeak& operator =(const JsWeak& obj) noexcept;
		JsWeak& operator =(JsWeak&& obj) noexcept;
		JsWeak(nullptr_t) noexcept;
		JsWeak& operator =(nullptr_t) noexcept;

	private:
		JsRawPersistent m_data;
	};

}