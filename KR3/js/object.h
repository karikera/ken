#pragma once


#include "type.h"
#include "object.h"
#include "class.h"
#include "persistent.h"

#include <KR3/data/linkedlist.h>

namespace kr
{
	namespace _pri_
	{
		class JsClassInfo:public Node<JsClassInfo, true>
		{
			friend _pri_::InternalTools;
			friend JsContext;
			friend JsRuntime;
		private:
			typedef JsObject* (*CTOR)(const JsArguments&);
			size_t m_index;
			byte m_classObjectBuffer[sizeof(JsClass)] alignas(JsClass);
			void (*m_initMethods)();
			void (*m_clearMethods)();
			Text16 m_name;
			CTOR m_ctor;
			size_t m_parentIndex;
			bool m_isGlobal;

		public:
			KRJS_EXPORT JsClassInfo(Text16 name, size_t parentIdx, CTOR ctor, void (*initMethods)(), void (*clearMethods)(), bool global) noexcept;
			static void operator delete(void* p) noexcept;

			size_t getIndex() noexcept;
			JsClass* get() noexcept;

			template <typename T>
			JsClassT<T>* get() noexcept
			{
				return (JsClassT<T>*)m_classObjectBuffer;
			}
		};
	}

	template <class Class, class Parent>
	class JsObjectT :public Parent
	{
		template <class Class, class Parent>
		friend class JsObjectT;
	private:
		static constexpr undefined_t className = undefined; // Need to define with Text16
		static _pri_::JsClassInfo s_classInfo;
		static JsObject* _ctor(const JsArguments &args) throws(JsException);
		static void _initMethods() noexcept;
		static void _clearMethods() noexcept;

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
		static void clearMethods() noexcept
		{
		}


		// 객체를 생성합니다
		static JsValue newInstanceRaw(JsArgumentsIn args) throws(JsException)
		{
			return classObject.newInstanceRaw(args);
		}
		virtual JsClass & getClass() noexcept override
		{
			return classObject;
		}
		virtual void finallize() noexcept override
		{
			delete static_cast<Class*>(this);
		}

		// 객체를 생성합니다
		// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
		template <typename ... ARGS> 
		static Class* newInstance(const ARGS & ... args) throws(JsException)
		{
			return classObject.newInstance(args ...);
		}
	};

	template <> class JsObjectT<JsObject> :public JsWeak
	{
		template <class Class, class Parent>
		friend class JsObjectT;
	private:
		static _pri_::JsClassInfo s_classInfo;

	public:
		static JsClassT<JsObject>& classObject;

		KRJS_EXPORT JsObjectT(const JsArguments & args) throws(JsObjectT);
		KRJS_EXPORT ~JsObjectT() noexcept;
		// 객체를 생성합니다
		static JsValue newInstanceRaw(JsArgumentsIn args) throws(JsObjectT);
		virtual JsClass& getClass() noexcept;
		virtual void finallize() noexcept;

		// 객체를 생성합니다
		// 기본적으로 Weak 상태로 생성되어, GC에 의하여 지워질 수 있습니다.
		template <typename ... ARGS> 
		static JsObject* newInstance(const ARGS & ... args) throws(JsException)
		{
			return classObject.newInstance(args ...);
		}
	};

	class JsObject :public JsObjectT<JsObject>
	{
		friend JsFunction;
	public:
		static constexpr const char16_t className[] = u"NativeObject";

		JsObject(const JsObject &) = delete;

		JsObject(const JsArguments & args) throws(JsException);
		~JsObject() noexcept;

		static void initMethods(JsClassT<JsObject>* cls) noexcept;
		KRJS_EXPORT bool deleted() noexcept;
	};

	ATTR_ANY _pri_::JsClassInfo JsObjectT<JsObject>::s_classInfo(JsObject::className, -1,
		[](const JsArguments& args) { return _new JsObject(args); }, 
		[] {}, [] {}, false);

	ATTR_ANY JsClassT<JsObject>& JsObjectT<JsObject>::classObject = *s_classInfo.get<JsObject>();

	// Need to define className to child of JsObjectT
	template <class Class, class Parent>
	_pri_::JsClassInfo JsObjectT<Class, Parent>::s_classInfo(
		Class::className, Parent::s_classInfo.getIndex(), _ctor, _initMethods, _clearMethods, Class::global);

	template <class Class, class Parent>
	JsClassT<Class>& JsObjectT<Class, Parent>::classObject = *s_classInfo.get<Class>();

	template <class Class, class Parent>
	JsObject* JsObjectT<Class, Parent>::_ctor(const JsArguments& args) throws(JsException)
	{
		return static_cast<JsObject*>(_newAligned(Class, args));
	}
	template <class Class, class Parent>
	void JsObjectT<Class, Parent>::_initMethods() noexcept
	{
		Class::initMethods(s_classInfo.get<Class>());
	}
	template <class Class, class Parent>
	void JsObjectT<Class, Parent>::_clearMethods() noexcept
	{
		Class::clearMethods();
	}

	class JsGetter :public Referencable<JsGetter>
	{
	public:
		virtual ~JsGetter() noexcept;
		virtual JsValue get(const JsValue& _this) noexcept = 0;

		template <typename GET>
		static JsGetter* wrap(GET&& get) noexcept;
	};
	class JsAccessor :public JsGetter
	{
	public:
		virtual ~JsAccessor() noexcept;
		virtual void set(const JsValue& _this, const JsValue& v) noexcept = 0;

		template <typename GET, typename SET>
		static JsAccessor* wrap(GET &&get, SET &&set) noexcept;
	};

	class JsIndexAccessor :public Referencable<JsIndexAccessor>
	{
	public:
		virtual void set(const JsValue& _this, uint32_t idx, const JsValue& v) noexcept = 0;
		virtual JsValue get(const JsValue& _this, uint32_t idx) noexcept = 0;

		template <typename GET, typename SET> 
		static JsIndexAccessor* wrap(GET &&get, SET &&set) noexcept;
		template <typename GET> 
		static JsIndexAccessor* wrap(GET &&get) noexcept;
	};

	namespace _pri_
	{
		template <typename GET, typename SET> class JsAccessorImpl :public JsAccessor
		{
		private:
			const GET m_get;
			const SET m_set;

		public:
			JsAccessorImpl(GET &&get, SET &&set) noexcept
				: m_get(move(get)), m_set(move(set))
			{
			}
			JsAccessorImpl(const GET& get, const SET& set) noexcept
				: m_get(get), m_set(set)
			{
			}
			~JsAccessorImpl() noexcept override
			{
			}
			virtual JsValue get(const JsValue& _this) noexcept override
			{
				return m_get(_this);
			}
			virtual void set(const JsValue& _this, const JsValue& v) noexcept override
			{
				return m_set(_this, v);
			}

		};
		template <typename GET> class JsGetterImpl :public JsGetter
		{
		private:
			const GET m_get;

		public:
			JsGetterImpl(const GET &get) noexcept
				: m_get(get)
			{
			}
			JsGetterImpl(GET &&get) noexcept
				: m_get(move(get))
			{
			}
			~JsGetterImpl() noexcept override
			{
			}
			virtual JsValue get(const JsValue& _this) noexcept override
			{
				return m_get(_this);
			}
		};

		template <typename GET, typename SET> class JsIndexAccessorGetSet :public JsIndexAccessor
		{
		private:
			const GET m_get;
			const SET m_set;

		public:
			JsIndexAccessorGetSet(const GET &get, const SET &set) noexcept
				:m_get(get), m_set(set)
			{
			}
			JsIndexAccessorGetSet(GET &&get, SET &&set) noexcept
				:m_get(move(get)), m_set(move(set))
			{
			}
			~JsIndexAccessorGetSet() noexcept override
			{
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
			JsIndexAccessorGet(GET &&get) noexcept
				:m_get(move(get))
			{
			}
			JsIndexAccessorGet(const GET &get) noexcept
				:m_get(get)
			{
			}
			~JsIndexAccessorGet() noexcept override
			{
			}
			virtual JsValue get(const JsValue& _this, uint32_t idx) noexcept override
			{
				return m_get(_this, idx);
			}
		};
	}

	template <typename GET, typename SET>
	JsAccessor* JsAccessor::wrap(GET &&get, SET &&set) noexcept
	{
		return new _pri_::JsAccessorImpl<decay_t<GET>, decay_t<SET> >(
			forward<GET>(get), forward<SET>(set));
	}
	template <typename GET>
	JsGetter* JsGetter::wrap(GET &&get) noexcept
	{
		return new _pri_::JsGetterImpl<decay_t<GET> >(forward<GET>(get));
	}

	template <typename GET, typename SET>
	JsIndexAccessor* JsIndexAccessor::wrap(GET &&get, SET &&set) noexcept
	{
		return new _pri_::JsIndexAccessorGetSet<decay_t<GET>, decay_t<SET> >(
			forward<GET>(get), forward<SET>(set));
	}
	template <typename GET>
	JsIndexAccessor* JsIndexAccessor::wrap(GET &&get) noexcept
	{
		return new _pri_::JsIndexAccessorGet<decay_t<GET> >(forward<GET>(get));
	}
}