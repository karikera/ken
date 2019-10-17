#pragma once


#include "type.h"
#include "external.h"
#include "object.h"
#include "class.h"

namespace kr
{
	namespace _pri_
	{
		class JsClassList;
		class JsClassInfo
		{
			friend _pri_::InternalTools;
			friend JsClassList;
			friend JsContext;
			friend JsRuntime;
		private:
			typedef JsObject* (*CTOR)(const JsArguments&);
			size_t m_index;
			byte m_classObjectBuffer[sizeof(JsClass)] alignas(JsClass);
			void (*m_initMethods)();
			Text16 m_name;
			CTOR m_ctor;
			size_t m_parentIndex;
			JsClassInfo* m_next;
			bool m_isGlobal;

		public:
			KRJS_EXPORT JsClassInfo(Text16 name, size_t parentIdx, CTOR ctor, void (*initMethods)(), bool global) noexcept;

			size_t getIndex() noexcept;
			JsClass* get() noexcept;
			JsClassInfo* next() noexcept;

			template <typename T>
			JsClassT<T>* get() noexcept
			{
				return (JsClassT<T>*)m_classObjectBuffer;
			}
		};
	}

	template <class Class, class Parent = JsObject>
	class JsObjectT :public Parent
	{
		template <class Class, class Parent>
		friend class JsObjectT;
	private:
		static constexpr undefined_t className = undefined; // Need to define with Text16
		static _pri_::JsClassInfo s_classInfo;
		static JsObject* _ctor(const JsArguments &args) throws(JsException);
		static void _initMethods() noexcept;

	public:
		static JsClassT<Class> &classObject;
		static constexpr bool global = true;

		JsObjectT(const JsArguments & args) :Parent(args)
		{
		}
		JsObjectT(const JsObjectT&) = delete;

		static void initMethods(JsClassT<Class> * cls) noexcept
		{
		}

		// 객체를 생성합니다
		static JsValue newInstance(JsArgumentsIn args)
		{
			return classObject.newInstance(args);
		}
		virtual JsClass & getClass() noexcept override
		{
			return classObject;
		}
		virtual void finallize() noexcept override
		{
			delete this;
		}

		// 객체를 생성합니다
		// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
		template <typename ... ARGS> 
		static Class* newInstance(const ARGS & ... args)
		{
			return classObject.newInstance(args ...);
		}
	};

	template <> class JsObjectT<JsObject> :public JsValue
	{
		template <class Class, class Parent>
		friend class JsObjectT;
	private:
		static _pri_::JsClassInfo s_classInfo;

	public:
		static JsClassT<JsObject>& classObject;

		JsObjectT(const JsArguments & args) throws(JsObjectT);
		~JsObjectT() noexcept;
		// 객체를 생성합니다
		static JsObject newInstanceRaw(JsArgumentsIn args);
		virtual JsClass& getClass() noexcept;
		virtual void finallize() noexcept;

		// 객체를 생성합니다
		// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
		template <typename ... ARGS> static JsObject* newInstance(const ARGS & ... args)
		{
			return classObject.newInstance(args ...);
		}
	};

	class JsObject :public JsObjectT<JsObject>
	{
		friend JsFunction;
	public:
		static constexpr char16_t className[] = u"NativeObject";

		JsObject(const JsObject &) = delete;

		JsObject(const JsArguments & args) throws(JsException);
		~JsObject() noexcept;

		static void initMethods(JsClassT<JsObject>* cls) noexcept;
		KRJS_EXPORT bool deleted() noexcept;
	};

	ATTR_ANY _pri_::JsClassInfo JsObjectT<JsObject>::s_classInfo(JsObject::className, -1,
		[](const JsArguments& args) { return _new JsObject(args); }, 
		[]{}, true);

	ATTR_ANY JsClassT<JsObject>& JsObjectT<JsObject>::classObject = *s_classInfo.get<JsObject>();

	// Need to define className to child of JsObjectT
	template <class Class, class Parent>
	_pri_::JsClassInfo JsObjectT<Class, Parent>::s_classInfo(
		Class::className, Parent::s_classInfo.getIndex(), _ctor, _initMethods, Class::global);

	template <class Class, class Parent>
	JsClassT<Class>& JsObjectT<Class, Parent>::classObject = *s_classInfo.get<Class>();

	template <class Class, class Parent>
	JsObject* JsObjectT<Class, Parent>::_ctor(const JsArguments& args) throws(JsException)
	{
		return static_cast<JsObject*>(_new Class(args));
	}
	template <class Class, class Parent>
	void JsObjectT<Class, Parent>::_initMethods() noexcept
	{
		Class::initMethods(s_classInfo.get<Class>());
	}

	class JsFilter :public JsExternal
	{
	private:
		int m_index;

	public:
		JsFilter(int index) noexcept
			: m_index(index)
		{
		}
		virtual JsValue filter(const JsValue & _v) noexcept = 0;

		template <typename FILTER>
		static JsFilter* wrap(int index, FILTER filter) noexcept;
	};

	class JsAccessor : public JsExternal
	{
	public:
		virtual void set(const JsObject& _this, const JsValue& v) noexcept = 0;
		virtual JsValue get(const JsObject& _this) noexcept = 0;


		template <typename GET, typename SET>
		JsAccessor* wrap(GET get, SET set) noexcept;
		template <typename GET>
		JsAccessor* wrap(GET get) noexcept;
	};

	class JsIndexAccessor : public JsExternal
	{
	public:
		virtual void set(const JsObject& _this, uint32_t idx, const JsValue& v) noexcept = 0;
		virtual JsValue get(const JsObject& _this, uint32_t idx) noexcept = 0;

		template <typename GET, typename SET> 
		JsIndexAccessor* wrap(GET get, SET set) noexcept;
		template <typename GET> 
		JsIndexAccessor* wrap(GET get) noexcept;
	};

	namespace _pri_
	{
		template <typename FILTER>
		class JsFilterImpl :public JsFilter
		{
		private:
			const FILTER m_filter;

		public:
			JsFilterImpl(int index, FILTER filter) noexcept
				: JsFilter(index), m_filter(std::move(filter))
			{
			}
			virtual void remove() noexcept override
			{
				delete this;
			}
			virtual JsValue filter(const JsValue& _v) noexcept override
			{
				return m_filter(_v);
			}

		};

		template <typename GET, typename SET> class JsAccessorGetSet :public JsAccessor
		{
		private:
			const GET m_get;
			const SET m_set;

		public:
			JsAccessorGetSet(GET get, SET set) noexcept
				: m_get(std::move(get)), m_set(std::move(set))
			{
			}
			virtual void remove() noexcept override
			{
				delete this;
			}
			virtual JsValue get(const JsObject& _this) noexcept override
			{
				return m_get(_this);
			}
			virtual void set(const JsObject& _this, const JsValue& v) noexcept override
			{
				return m_set(_this, v);
			}

		};
		template <typename GET> class JsAccessorGet :public JsAccessor
		{
		private:
			const GET m_get;

		public:
			JsAccessorGet(GET get) noexcept
				: m_get(std::move(get))
			{
			}
			virtual void remove() noexcept override
			{
				delete this;
			}
			virtual JsValue get(const JsObject& _this) noexcept override
			{
				return m_get(_this);
			}
			virtual void set(const JsObject& _this, const JsValue& v) noexcept override
			{
			}
		};

		template <typename GET, typename SET> class JsIndexAccessorGetSet :public JsIndexAccessor
		{
		private:
			const GET m_get;
			const SET m_set;

		public:
			JsIndexAccessorGetSet(GET get, SET set) noexcept
				:m_get(std::move(get)), m_set(std::move(set))
			{
			}
			virtual void remove() noexcept override
			{
				delete this;
			}
			virtual JsValue get(const JsObject& _this, uint32_t idx) noexcept override
			{
				return m_get(_this, idx);
			}
			virtual void set(const JsObject& _this, uint32_t idx, const JsValue& v) noexcept override
			{
				return m_set(_this, idx, v);
			}

		};

		template <typename GET> class JsIndexAccessorGet :public JsIndexAccessor
		{
		private:
			const GET m_get;

		public:
			JsIndexAccessorGet(GET get) noexcept
				:m_get(std::move(get))
			{
			}
			virtual void remove() noexcept override
			{
				delete this;
			}
			virtual JsValue get(const JsObject& _this, uint32_t idx) noexcept override
			{
				return m_get(_this, idx);
			}
			virtual void set(const JsObject& _this, uint32_t idx, const JsValue& v) noexcept override
			{
			}
		};
	}

	template <typename FILTER>
	static JsFilter* JsFilter::wrap(int index, FILTER filter) noexcept
	{
		return new _pri_::JsFilterImpl<FILTER>(index, std::move(filter));
	}


	template <typename GET, typename SET>
	JsAccessor* JsAccessor::wrap(GET get, SET set) noexcept
	{
		return new _pri_::JsAccessorGetSet<GET, SET>(std::move(get), std::move(set));
	}
	template <typename GET>
	JsAccessor* JsAccessor::wrap(GET get) noexcept
	{
		return new _pri_::JsAccessorGet<GET>(std::move(get));
	}

	template <typename GET, typename SET>
	JsIndexAccessor* JsIndexAccessor::wrap(GET get, SET set) noexcept
	{
		return new _pri_::JsIndexAccessorGetSet<GET, SET>(std::move(get), std::move(set));
	}
	template <typename GET>
	JsIndexAccessor* JsIndexAccessor::wrap(GET get) noexcept
	{
		return new _pri_::JsIndexAccessorGet<GET>(std::move(get));
	}
}