#pragma once

#include "type.h"


namespace kr
{
	template <>
	class V8Handle<v8::Data>
	{
		friend V8Function;
		friend _pri_::InternalTools;
	public:
		KR_EASYV8_DLLEXPORT V8Handle();

		KR_EASYV8_DLLEXPORT ~V8Handle();

		KR_EASYV8_DLLEXPORT V8Handle(const V8Handle & value);

		KR_EASYV8_DLLEXPORT V8Handle(V8Handle && value);

		// ��ü�� ���ϴ�.
		KR_EASYV8_DLLEXPORT void setEmpty();

		// �� ��ü�� �ִ��� Ȯ���մϴ�.
		KR_EASYV8_DLLEXPORT bool isEmpty() const;

		// �� ������ �����־, ��ü�� GC�� ���Ͽ� ������ �� �ְ� ����ϴ�.
		KR_EASYV8_DLLEXPORT void setWeak();

		template <typename LAMBDA>
		void setWeak(LAMBDA lambda);

		// setWeak�� ���Ͽ�, GC�� ���ؼ� ������ �� �ְ� �� ���¸� �������� �ٲߴϴ�.
		KR_EASYV8_DLLEXPORT void clearWeak();

		// �ٸ� ��ü�� ���� �񱳸� �մϴ�.
		KR_EASYV8_DLLEXPORT bool equals(const V8Handle& o) const;

		inline V8Handle & operator =(const V8Handle & value);
		inline bool operator ==(const V8Handle & o) const;
		inline bool operator !=(const V8Handle & o) const;
	protected:
		v8::Persistent<v8::Data> m_value;

	private:
		// �� ������ �����־, ��ü�� GC�� ���Ͽ� ������ �� �ְ� ����ϴ�.
		// ��ü�� ������ �� callback �� ȣ��˴ϴ�.
		KR_EASYV8_DLLEXPORT void _setWeak(JsLambda callback);
	};

	template <>
	class V8Handle<v8::Private> :public V8Data
	{
		friend V8Object;
		friend _pri_::InternalTools;
	public:
		KR_EASYV8_DLLEXPORT V8Handle();
		KR_EASYV8_DLLEXPORT ~V8Handle();
		inline V8Handle(nullptr_t)
			:V8Data()
		{
		}
		inline V8Handle(const V8Handle & value)
			:V8Data(value)
		{
		}


	private:
		inline v8::Persistent<v8::Private>& _handle();
		inline const v8::Persistent<v8::Private>& _handle() const;
	};
	template <>
	class V8Handle<v8::Value>:public V8Data
	{
		friend _pri_::InternalTools;
		friend JsString;
	public:
		// ���� Ÿ�� Ȯ��
		template <typename T> bool is() const;

		// ��ü���� Ȯ��
		template <> KR_EASYV8_DLLEXPORT bool is<v8::Object>() const;

		// �Լ����� Ȯ��
		template <> KR_EASYV8_DLLEXPORT bool is<v8::Function>() const;

		// �迭���� Ȯ��
		template <> KR_EASYV8_DLLEXPORT bool is<v8::Array>() const;

		// ��Ʈ������ �޾ƿ���
		KR_EASYV8_DLLEXPORT JsString toString() const;

	protected:
		template <typename T>
		inline void _set(const V8Handle & any);
	};

	// V8 ���� ��ü�� ����
	// V8 ��ü
	template <>
	class V8Handle<v8::Object> : public V8Handle<v8::Value>
	{
		friend _pri_::InternalTools;
		friend V8Function;
		friend JsAny;
		friend V8Array;
		friend V8Class;
		friend JsContext;
		friend NativeObject;

	public:
		inline V8Handle();
		inline V8Handle(nullptr_t);
		inline V8Handle(const V8Handle &obj);
		
		inline explicit V8Handle(const V8Value & any);

		// ��ü �Ҵ�
		KR_EASYV8_DLLEXPORT static V8Handle<v8::Object> newInstance();

		// ��ü�� Ŭ������ ����ֽ��ϴ�
		// ����: JsException
		KR_EASYV8_DLLEXPORT void setClass(JsText name, V8Handle<v8::FunctionTemplate> * value);

		// ��ü�� ������ ����ֽ��ϴ�
		KR_EASYV8_DLLEXPORT void set(JsText name, JsAny value);

		// ��ü�� ������ �����ɴϴ�
		KR_EASYV8_DLLEXPORT JsAny get(JsText name) const;

		// ��ü���� ������ ����ϴ�.
		KR_EASYV8_DLLEXPORT bool remove(JsText name) const;

		// ���� ������ ����ֽ��ϴ�
		KR_EASYV8_DLLEXPORT void set(const V8Private & p, JsAny value);

		// ���� ������ �����ɴϴ�
		KR_EASYV8_DLLEXPORT JsAny get(const V8Private & p) const;

		// ���� ������ ����ϴ�.
		KR_EASYV8_DLLEXPORT bool remove(const V8Private & p) const;

		// �迭�� ������ ����ֽ��ϴ�
		KR_EASYV8_DLLEXPORT void set(uint32_t idx, JsAny value);

		// �迭�� ������ �����ɴϴ�
		KR_EASYV8_DLLEXPORT JsAny get(uint32_t idx) const;
		
		KR_EASYV8_DLLEXPORT void setInternal(int idx, JsAny value) const;
		KR_EASYV8_DLLEXPORT JsAny getInternal(int idx) const;
		KR_EASYV8_DLLEXPORT int internalFieldCount() const;

		// ����Ƽ�� ��ü�� �����ɴϴ�
		// ����Ƽ�� ��ü�� �ƴϸ�, NULL�� ���ɴϴ�
		KR_EASYV8_DLLEXPORT NativeObject * getNativeObject() const;

		// ���� ������ ����ֽ��ϴ�
		template <typename T>
		inline void set(const JsField<T> & p, T value)
		{
			return setInternal(p.getIndex(), (JsAny)value);
		}

		// ���� ������ �����ɴϴ�
		template <typename T>
		inline T get(const JsField<T> & p) const
		{
			return getInternal(p.getIndex()).cast<T>();
		}

		// ����Ƽ�� ��ü�� Ÿ���� ���ؼ� �����ɴϴ�
		// �ش� Ÿ�԰� ���� ������ NULL�� ���ɴϴ�
		template <typename T>
		inline T * getNativeObject() const
		{
			return dynamic_cast<T*>(getNativeObject());
		}

#ifdef __KR3_INCLUDED
		// ��ü�� Ŭ������ ����ֽ��ϴ�
		// ����: JsException
		void setClass(Text16 name, V8Handle<v8::FunctionTemplate>* value)
		{
			return setClass(JsText::from(name), value);
		}

		// ��ü�� ������ ����ֽ��ϴ�
		void set(Text16 name, JsAny value)
		{
			return set(JsText::from(name));
		}

		// ��ü�� ������ �����ɴϴ�
		JsAny get(Text16 name) const
		{
			return get(JsText::from(name));
		}

		// ��ü���� ������ ����ϴ�.
		bool remove(Text16 name) const
		{
			return remove(JsText::from(name));
		}


		// ��ü�� �Լ��� ȣ���մϴ�
		// JsAny �����͸� �̿��Ͽ� �Ķ���͸� �ѱ�ϴ�
		inline JsAny callRaw(Text16 name, JsArgumentsIn args) const;

		// ��ü�� Ŭ������ ����ֽ��ϴ�
		template <typename Class> inline void setClass();

		// ��ü�� �Լ��� ����ֽ��ϴ�
		// const JsArguments & �� �̿��Ͽ� �Ķ���͸� �޽��ϴ�
		template <typename LAMBDA> inline void setFunctionRaw(Text16 name, LAMBDA lambda);

		// ��ü�� �Լ��� ȣ���մϴ�.
		template <typename RET, typename ... ARGS> inline RET call(Text16 name, const ARGS & ... args) const;

		// ��ü�� �Լ��� ����ֽ��ϴ�.
		// ���ο��� �Ķ���� Ÿ�Կ� ���߾� ĳ���� �˴ϴ�.
		template <typename LAMBDA> inline void setFunction(Text16 name, LAMBDA lambda);
#endif
	};


	// V8 ���� ��ü�� ����
	// V8 �迭
	template <>
	class V8Handle<v8::Array> : public V8Object
	{
		friend _pri_::InternalTools;
		friend JsAny;
		friend V8Object;
		friend V8Class;
		friend JsContext;
		friend NativeObject;
		
	public:
		inline V8Handle();
		inline V8Handle(std::nullptr_t);
		inline V8Handle(const V8Handle &obj);
		inline explicit V8Handle(const V8Value & any);

		// �迭 �Ҵ�
		KR_EASYV8_DLLEXPORT static V8Handle<v8::Array> newInstance(uint32_t length = 0);

		// �迭 ����
		KR_EASYV8_DLLEXPORT uint32_t getLength() const;

#ifdef __KR3_INCLUDED

		// ��ü�� �Լ��� ȣ���մϴ�.
		// JsAny �����͸� �̿��Ͽ� �Ķ���͸� �ѱ�ϴ�.
		inline JsAny call(Text16 name, JsArgumentsIn args) const;

		// ��ü�� Ŭ������ ����ֽ��ϴ�.
		template <typename Class> inline void setClass();

		// ��ü�� �Լ��� ����ֽ��ϴ�.
		// const JsArguments & �� �̿��Ͽ� �Ķ���͸� �޽��ϴ�.
		template <typename LAMBDA> inline void setFunctionRaw(Text16 name, LAMBDA lambda);

		// ��ü�� �Լ��� ȣ���մϴ�.
		template <typename RET, typename ... ARGS> inline RET call(Text16 name, const ARGS & ... args) const;

		// ��ü�� �Լ��� ����ֽ��ϴ�.
		// ���ο��� �Ķ���� Ÿ�Կ� ���߾� ĳ���� �˴ϴ�.
		template <typename LAMBDA> inline void setFunction(Text16 name, LAMBDA lambda);
#endif
	};

	// V8 ���� ��ü�� ����
	// V8 �Լ�
	template <>
	class V8Handle<v8::Function> : public V8Object
	{
		friend _pri_::InternalTools;
		friend JsAny;
		friend V8Object;

	public:
		inline V8Handle();
		inline V8Handle(std::nullptr_t);
		inline V8Handle(const V8Handle &obj);
		inline explicit V8Handle(const V8Value & any);
		
		KR_EASYV8_DLLEXPORT JsAny callRaw(const V8Object &_this, JsArgumentsIn args) const;
		KR_EASYV8_DLLEXPORT V8Object callNewRaw(JsArgumentsIn args) const;

		// ��ü�� �Լ��� ȣ���մϴ�.
		template <typename RET, typename T, typename ... ARGS> inline RET call(T* _this, const ARGS & ... args) const;
		// ��ü�� �Լ��� ȣ���մϴ�.
		// ��ü�� ��������� undefined�� ��ȯ�մϴ�.
		template <typename RET, typename T, typename ... ARGS> inline RET safeCall(T* _this, const ARGS & ... args) const;
		// ��ü�� �Լ��� ȣ���մϴ�.
		template <typename ... ARGS> inline V8Object callNew(const ARGS & ... args) const;
	};
}