#pragma once

#include "function.h"

namespace kr
{

	// �ڹٽ�ũ��Ʈ ����Ƽ�� Ŭ����
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

		// Ŭ������ ��� ������ �����Ѵ�
		// ����: JsException (�̸��� �ߺ��� �� �߻�)
		KRJS_EXPORT void setField(Text16 _name, const JsValue& v) throws(JsException);

		KRJS_EXPORT void makeField(Text16 _name, JsFilter* _filter) noexcept;
		KRJS_EXPORT void makeReadOnlyField(Text16 _name, int index) noexcept;
		KRJS_EXPORT void setAccessor(Text16 _name, JsAccessor* _accessor) noexcept;
		KRJS_EXPORT void setReadOnlyAccessor(Text16 _name, JsAccessor* _accessor) noexcept;
		KRJS_EXPORT void setIndexAccessor(JsIndexAccessor* _accessor) noexcept;
		KRJS_EXPORT void setReadOnlyIndexAccessor(JsIndexAccessor* _accessor) noexcept;

		// Ŭ������ ��� ������ �����Ѵ�
		// ����: JsException (�̸��� �ߺ��� �� �߻�)
		KRJS_EXPORT void setStaticField(Text16 _name, const JsValue &v) noexcept;

		KRJS_EXPORT void setStaticAccessor(Text16 _name, JsAccessor* _accessor) noexcept;
		KRJS_EXPORT void setStaticReadOnlyAccessor(Text16 _name, JsAccessor* _accessor) noexcept;

		// ��ü�� �����մϴ�
		KRJS_EXPORT JsValue newInstanceRaw(JsArgumentsIn args) const throws(JsException);

		// ��ü�� �����մϴ�
		// �⺻������ Weak ���·� �����Ǿ�, GC�� ���Ͽ� ������ �� �ֽ��ϴ�
		KRJS_EXPORT JsObject* newInstanceRawPtr(JsArgumentsIn args) const throws(JsException);

		template <typename P>
		void setStaticAccessor(Text16 _name, P* value) noexcept
		{
			setStaticAccessor(_name,
				[value](JsValue)->JsValue { return *value; },
				[value](JsValue, JsValue nv) { *value = nv.get<P>(); }
			);
		}

		template <typename P>
		void setStaticReadOnlyAccessor(Text16 _name, P* value) noexcept
		{
			setStaticReadOnlyAccessor(_name,
				[value](JsValue)->JsValue { return *value; }
			);
		}

		template <typename LAMBDA>
		void setMethod(Text16 _name, LAMBDA _lambda) noexcept
		{
			set(_name, JsFunction(_lambda));
		}

		template <typename GET, typename SET>
		void setAccessor(Text16 _name, GET get, SET set) noexcept
		{
			setAccessor(_name, JsAccessor(get, set));
		}

		template <typename GET>
		void setReadOnlyAccessor(Text16 _name, GET get) noexcept
		{
			setReadOnlyAccessor(_name, JsAccessor(get));
		}

		template <typename GET, typename SET>
		void setStaticAccessor(Text16 _name, GET get, SET set) noexcept
		{
			setStaticAccessor(_name, JsAccessor(get, set));
		}

		template <typename GET>
		void setStaticReadOnlyAccessor(Text16 _name, GET get) noexcept
		{
			setStaticReadOnlyAccessor(_name, JsAccessor(get));
		}


		template <typename LAMBDA>
		void setStaticMethodRaw(Text16 _name, LAMBDA _lambda) noexcept;

		template <typename T, typename P>
		void setMethodAccessor(Text16 _name, P(T::* get)()) noexcept;
		template <typename T, typename P>
		void setMethodAccessor(Text16 _name, P(T::* get)(), void(T::* set)(P)) noexcept;

		template <typename T, typename RET, typename ... ARGS>
		void setMethod(Text16 _name, RET(T::* func)(ARGS ...)) noexcept;

		template <typename LAMBDA>
		void setStaticMethod(Text16 _name, LAMBDA _lambda) noexcept;


	};

	// �ڹٽ�ũ��Ʈ ����Ƽ�� Ŭ����
	// C++ Ŭ������ ���� ����
	// C++ Ŭ������ initMethods �Լ��� �����ؾ��Ѵ�.
	template <typename T> class JsClassT :public JsClass
	{
	public:
		JsClassT() = delete;
		JsClass createChild() noexcept;
		
		template <typename P>
		void setAccessor(Text16 _name, P(T::* v)) noexcept;

		template <typename P>
		void setReadOnlyAccessor(Text16 _name, P(T::* v)) noexcept;

		template <typename P>
		void setMethodAccessor(Text16 _name, P(T::* get)()) noexcept;

		template <typename P>
		void setMethodAccessor(Text16 _name, P(T::* get)(), void(T::* set)(P)) noexcept;

		template <typename P>
		void setNullableMethodAccessor(Text16 _name, P(T::* get)()) noexcept;

		template <typename P>
		void setNullableMethodAccessor(Text16 _name, P(T::* get)(), void(T::* set)(P)) noexcept;

		void setMethodRaw(Text16 _name, JsValue(T::* func)(const JsArguments&)) noexcept;

		template <typename RET, typename ... ARGS>
		void setMethod(Text16 _name, RET(T::* func)(ARGS ...)) noexcept;

		template <typename P>
		void setIndexAccessor(P(T::* get)(uint32_t)) noexcept;

		template <typename P>
		void setIndexAccessor(P(T::* get)(uint32_t), void(T::* set)(uint32_t, P)) noexcept;

		// ��ü�� �����մϴ�
		// �⺻������ Weak ���·� �����Ǿ�, GC�� ���Ͽ� ������ �� �ֽ��ϴ�.
		template <typename ... ARGS> T* newInstance(const ARGS& ... args) const noexcept;
	};

}