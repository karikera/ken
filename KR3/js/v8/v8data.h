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

		// 객체를 비웁니다.
		KR_EASYV8_DLLEXPORT void setEmpty();

		// 이 객체가 있는지 확인합니다.
		KR_EASYV8_DLLEXPORT bool isEmpty() const;

		// 이 변수가 남아있어도, 객체가 GC에 의하여 지워질 수 있게 만듭니다.
		KR_EASYV8_DLLEXPORT void setWeak();

		template <typename LAMBDA>
		void setWeak(LAMBDA lambda);

		// setWeak에 의하여, GC에 의해서 지워질 수 있게 된 상태를 원래데로 바꿉니다.
		KR_EASYV8_DLLEXPORT void clearWeak();

		// 다른 객체와 참조 비교를 합니다.
		KR_EASYV8_DLLEXPORT bool equals(const V8Handle& o) const;

		inline V8Handle & operator =(const V8Handle & value);
		inline bool operator ==(const V8Handle & o) const;
		inline bool operator !=(const V8Handle & o) const;
	protected:
		v8::Persistent<v8::Data> m_value;

	private:
		// 이 변수가 남아있어도, 객체가 GC에 의하여 지워질 수 있게 만듭니다.
		// 객체가 지워질 때 callback 이 호출됩니다.
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
		// 내부 타입 확인
		template <typename T> bool is() const;

		// 객체인지 확인
		template <> KR_EASYV8_DLLEXPORT bool is<v8::Object>() const;

		// 함수인지 확인
		template <> KR_EASYV8_DLLEXPORT bool is<v8::Function>() const;

		// 배열인지 확인
		template <> KR_EASYV8_DLLEXPORT bool is<v8::Array>() const;

		// 스트링으로 받아오기
		KR_EASYV8_DLLEXPORT JsString toString() const;

	protected:
		template <typename T>
		inline void _set(const V8Handle & any);
	};

	// V8 내부 객체의 참조
	// V8 객체
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

		// 객체 할당
		KR_EASYV8_DLLEXPORT static V8Handle<v8::Object> newInstance();

		// 객체에 클래스를 집어넣습니다
		// 예외: JsException
		KR_EASYV8_DLLEXPORT void setClass(JsText name, V8Handle<v8::FunctionTemplate> * value);

		// 객체에 변수를 집어넣습니다
		KR_EASYV8_DLLEXPORT void set(JsText name, JsAny value);

		// 객체에 변수를 가져옵니다
		KR_EASYV8_DLLEXPORT JsAny get(JsText name) const;

		// 객체에서 변수를 지웁니다.
		KR_EASYV8_DLLEXPORT bool remove(JsText name) const;

		// 내부 변수를 집어넣습니다
		KR_EASYV8_DLLEXPORT void set(const V8Private & p, JsAny value);

		// 내부 변수를 가져옵니다
		KR_EASYV8_DLLEXPORT JsAny get(const V8Private & p) const;

		// 내부 변수를 지웁니다.
		KR_EASYV8_DLLEXPORT bool remove(const V8Private & p) const;

		// 배열에 변수를 집어넣습니다
		KR_EASYV8_DLLEXPORT void set(uint32_t idx, JsAny value);

		// 배열에 변수를 가져옵니다
		KR_EASYV8_DLLEXPORT JsAny get(uint32_t idx) const;
		
		KR_EASYV8_DLLEXPORT void setInternal(int idx, JsAny value) const;
		KR_EASYV8_DLLEXPORT JsAny getInternal(int idx) const;
		KR_EASYV8_DLLEXPORT int internalFieldCount() const;

		// 네이티브 객체를 가져옵니다
		// 네이티브 객체가 아니면, NULL이 나옵니다
		KR_EASYV8_DLLEXPORT NativeObject * getNativeObject() const;

		// 내부 변수를 집어넣습니다
		template <typename T>
		inline void set(const JsField<T> & p, T value)
		{
			return setInternal(p.getIndex(), (JsAny)value);
		}

		// 내부 변수를 가져옵니다
		template <typename T>
		inline T get(const JsField<T> & p) const
		{
			return getInternal(p.getIndex()).cast<T>();
		}

		// 네이티브 객체를 타입을 정해서 가져옵니다
		// 해당 타입과 맞지 않으면 NULL이 나옵니다
		template <typename T>
		inline T * getNativeObject() const
		{
			return dynamic_cast<T*>(getNativeObject());
		}

#ifdef __KR3_INCLUDED
		// 객체에 클래스를 집어넣습니다
		// 예외: JsException
		void setClass(Text16 name, V8Handle<v8::FunctionTemplate>* value)
		{
			return setClass(JsText::from(name), value);
		}

		// 객체에 변수를 집어넣습니다
		void set(Text16 name, JsAny value)
		{
			return set(JsText::from(name));
		}

		// 객체에 변수를 가져옵니다
		JsAny get(Text16 name) const
		{
			return get(JsText::from(name));
		}

		// 객체에서 변수를 지웁니다.
		bool remove(Text16 name) const
		{
			return remove(JsText::from(name));
		}


		// 객체에 함수를 호출합니다
		// JsAny 포인터를 이용하여 파라미터를 넘깁니다
		inline JsAny callRaw(Text16 name, JsArgumentsIn args) const;

		// 객체에 클래스를 집어넣습니다
		template <typename Class> inline void setClass();

		// 객체에 함수를 집어넣습니다
		// const JsArguments & 를 이용하여 파라미터를 받습니다
		template <typename LAMBDA> inline void setFunctionRaw(Text16 name, LAMBDA lambda);

		// 객체의 함수를 호출합니다.
		template <typename RET, typename ... ARGS> inline RET call(Text16 name, const ARGS & ... args) const;

		// 객체에 함수를 집어넣습니다.
		// 내부에서 파라미터 타입에 맞추어 캐스팅 됩니다.
		template <typename LAMBDA> inline void setFunction(Text16 name, LAMBDA lambda);
#endif
	};


	// V8 내부 객체의 참조
	// V8 배열
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

		// 배열 할당
		KR_EASYV8_DLLEXPORT static V8Handle<v8::Array> newInstance(uint32_t length = 0);

		// 배열 길이
		KR_EASYV8_DLLEXPORT uint32_t getLength() const;

#ifdef __KR3_INCLUDED

		// 객체에 함수를 호출합니다.
		// JsAny 포인터를 이용하여 파라미터를 넘깁니다.
		inline JsAny call(Text16 name, JsArgumentsIn args) const;

		// 객체에 클래스를 집어넣습니다.
		template <typename Class> inline void setClass();

		// 객체에 함수를 집어넣습니다.
		// const JsArguments & 를 이용하여 파라미터를 받습니다.
		template <typename LAMBDA> inline void setFunctionRaw(Text16 name, LAMBDA lambda);

		// 객체의 함수를 호출합니다.
		template <typename RET, typename ... ARGS> inline RET call(Text16 name, const ARGS & ... args) const;

		// 객체에 함수를 집어넣습니다.
		// 내부에서 파라미터 타입에 맞추어 캐스팅 됩니다.
		template <typename LAMBDA> inline void setFunction(Text16 name, LAMBDA lambda);
#endif
	};

	// V8 내부 객체의 참조
	// V8 함수
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

		// 객체의 함수를 호출합니다.
		template <typename RET, typename T, typename ... ARGS> inline RET call(T* _this, const ARGS & ... args) const;
		// 객체의 함수를 호출합니다.
		// 객체가 비어있으면 undefined를 반환합니다.
		template <typename RET, typename T, typename ... ARGS> inline RET safeCall(T* _this, const ARGS & ... args) const;
		// 객체의 함수를 호출합니다.
		template <typename ... ARGS> inline V8Object callNew(const ARGS & ... args) const;
	};
}