#pragma once

#include "function.h"

namespace kr
{

	// 자바스크립트 네이티브 클래스
	class JsClass :public JsValue
	{
		friend JsContext;
		friend JsObject;
		friend _pri_::InternalTools;

	private:
		typedef JsObject* (*CTOR)(const JsArguments&);
		KRJS_EXPORT JsClass _createChild(Text16 _name, CTOR ctor) const noexcept;
		
	public:
		JsClass() noexcept;
		JsClass(const JsRawData& _copy) noexcept;
		~JsClass() noexcept;

		template <typename T>
		JsClass createChild(Text16 _name = nullptr) noexcept;

		KRJS_EXPORT void setField(const JsPropertyId &name, const JsValue& v) throws(JsException);
		KRJS_EXPORT void setField(const JsValue &name, const JsValue& v) throws(JsException);
		template <size_t size>
		void setField(const char16(&name)[size], const JsValue& v) throws(JsException)
		{
			setField((JsPropertyId)name, v);
		}

		KRJS_EXPORT void setAccessorRaw(const JsPropertyId& name, JsAccessor* _accessor) noexcept;
		KRJS_EXPORT void setGetterRaw(const JsPropertyId& name, JsGetter* _accessor) noexcept;
		KRJS_EXPORT void setIndexAccessorRaw(JsIndexAccessor* _accessor) noexcept;
		KRJS_EXPORT void setReadOnlyIndexAccessorRaw(JsIndexAccessor* _accessor) noexcept;
		
		KRJS_EXPORT void setStaticAccessorRaw(const JsPropertyId& name, JsAccessor* _accessor) noexcept;
		KRJS_EXPORT void setStaticGetterRaw(const JsPropertyId& name, JsGetter* _accessor) noexcept;

		// 객체를 생성합니다
		KRJS_EXPORT JsValue newInstanceRaw(JsArgumentsIn args) const throws(JsException);

		template <typename P>
		void setStaticAccessor(const JsPropertyId& name, P* value) noexcept
		{
			setMethodStaticAccessor(name,
				[value](JsValue)->JsValue { return *value; },
				[value](JsValue, JsValue nv) { *value = nv.get<P>(); }
			);
		}

		template <typename P>
		void setStaticGetter(const JsPropertyId& name, P* value) noexcept
		{
			setStaticGetterL(name,
				[value](JsValue)->JsValue { return *value; }
			);
		}

		template <typename GET, typename SET>
		void setAccessorL(const JsPropertyId& name, GET&& get, SET&& set) noexcept
		{
			setAccessorRaw(name, JsAccessor::wrap(forward<GET>(get), forward<SET>(set)));
		}

		template <typename GET>
		void setGetterL(const JsPropertyId& name, GET &&get) noexcept
		{
			setGetterRaw(name, JsGetter::wrap(forward<GET>(get)));
		}

		template <typename GET, typename SET>
		void setStaticAccessorL(const JsPropertyId& name, GET &&get, SET &&set) noexcept
		{
			setStaticAccessorRaw(name, JsAccessor::wrap(forward<GET>(get), forward<SET>(set)));
		}

		template <typename GET>
		void setStaticGetterL(const JsPropertyId &name, GET &&get) noexcept
		{
			setStaticGetterRaw(name, JsGetter::wrap(forward<GET>(get)));
		}

		template <typename T, typename P>
		void setMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept;

		template <typename T, typename P>
		void setMethodAccessor(const JsPropertyId& name, P(T::* get)(), void(T::* set)(P)) noexcept;

		template <typename LAMBDA>
		void setMethodRaw(const JsPropertyId &name, LAMBDA&& _lambda) noexcept
		{
			setField(name, JsFunction::make(forward<LAMBDA>(_lambda)));
		}

		template <typename LAMBDA>
		void setStaticMethodRaw(const JsPropertyId& name, LAMBDA&& _lambda) noexcept;

		template <typename T, typename RET, typename ... ARGS>
		void setMethod(const JsPropertyId& name, RET(T::* func)(ARGS ...)) noexcept;

		template <typename LAMBDA>
		void setStaticMethod(const JsPropertyId& name, LAMBDA&& lambda) noexcept;

	};

	// 자바스크립트 네이티브 클래스
	// C++ 클래스로 부터 생성
	// C++ 클래스는 initMethods 함수를 구현해야한다.
	template <typename T> class JsClassT :public JsClass
	{
	public:
		JsClassT() = delete;
		JsClass createChild() noexcept;
		
		template <typename P>
		void setAccessor(const JsPropertyId& name, P(T::* v)) noexcept;

		template <typename P>
		void setGetter(const JsPropertyId& name, P(T::* v)) noexcept;

		template <typename P>
		void setMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept;

		template <typename P>
		void setMethodAccessor(const JsPropertyId& name, P(T::* get)(), void(T::* set)(P)) noexcept;

		template <typename P>
		void setNullableMethodGetter(const JsPropertyId& name, P(T::* get)()) noexcept;

		template <typename P>
		void setNullableMethodAccessor(const JsPropertyId& name, P(T::* get)(), void(T::* set)(P)) noexcept;

		void setMethodRaw(const JsPropertyId &name, JsValue(T::* func)(const JsArguments&)) noexcept;

		template <typename RET, typename ... ARGS>
		void setMethod(const JsPropertyId& name, RET(T::* func)(ARGS ...)) noexcept;

		template <typename P>
		void setIndexAccessor(P(T::* get)(uint32_t)) noexcept;

		template <typename P>
		void setIndexAccessor(P(T::* get)(uint32_t), void(T::* set)(uint32_t, P)) noexcept;

		// 객체를 생성합니다
		// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
		template <typename ... ARGS> T* newInstance(const ARGS& ... args) const throws(JsException);
	};

}