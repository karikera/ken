#pragma once

#include "type.h"

#include "../meta/value.h"

#include <assert.h>
#include <string.h>
#include <atomic>

#ifdef WIN32
typedef struct IUnknown IUnknown;
#endif

namespace kr
{
	template <class T> class Manual;
	template <class T> class Keep;
	template <typename T = Empty> class Interface;
	template <typename T> class Counter;
	template <typename T> using Reference = Keep<Counter<T>>;
	template <typename T> class StateKeeper;

#define localPointer(v)			::kr::Keep<pdecltype(v)> UNIQUE(AUTO_RELEASE)(v)
#define localState(type, ...)	::kr::StateKeeper<type> UNIQUE(STATE_KEEPER)(_VA_ARGS_)

	namespace _pri_
	{
		meta::bool_false isInterfaceHelper(void *);
		template <typename T>
		meta::bool_true isInterfaceHelper(Interface<T> *);
		template <bool v>
		struct IfInterfaceCallImpl;
		template <>
		struct IfInterfaceCallImpl<true>
		{
			template <typename T>
			inline static void addRef(T * p) noexcept
			{
				p->AddRef();
			}
			template <typename T>
			inline static void release(T * p) noexcept
			{
				p->Release();
			}
		};
		template <>
		struct IfInterfaceCallImpl<false>
		{
			template <typename T>
			inline static void addRef(T * p) noexcept
			{
			}
			template <typename T>
			inline static void release(T * p) noexcept
			{
				delete p;
			}
		};
	}

	template <typename T>
	struct IsInterface
	{
		static constexpr bool value =
#ifdef WIN32
			std::is_base_of<IUnknown, T>::value ||
#endif
			decltype(_pri_::isInterfaceHelper((T*)nullptr))::value;
	};
	template <typename T>
	struct IfInterfaceCall: _pri_::IfInterfaceCallImpl<IsInterface<T>::value>
	{
	};	

	struct Releaser
	{
		template <typename T>
		inline static void addRefFirst(T * value) noexcept
		{
			IfInterfaceCall<T>::addRef(value);
		}
		template <typename T>
		inline static void addRef(T * value) noexcept
		{
			IfInterfaceCall<T>::addRef(value);
		}
		template <typename T>
		inline static void release(T * value) noexcept
		{
			IfInterfaceCall<T>::release(value);
		}
	};

	/*
	T를 수동으로 생성, 소멸시키는 클래스 래퍼
	create로 생성시키며,
	remove로 소멸시킨다.
	*/
	template <class T>
	class alignas(alignof(T)) Manual
	{
	public:
		Manual() noexcept;
		template <typename ... ARGS> void create(const ARGS & ... args);
		void remove() noexcept;
		T* address() noexcept;
		T* operator ->() noexcept;
		operator T*() noexcept;
		const T* operator ->() const noexcept;
		operator const T*() const noexcept;

	private:
		uint8_t m_buffer[sizeof(T)];
		ondebug(bool m_constructed);
	};

	/*
	T를 수동으로 생성, 소멸시키는 클래스 래퍼
	create로 생성시키며,
	remove로 소멸시킨다.
	*/
	template <typename T>
	class alignas(alignof(T)) Deferred
	{
	public:
		template <typename ... ARGS>
		void create(const ARGS & ... args) noexcept;
		void remove() noexcept;
		operator T*() noexcept;
		T* operator ->() noexcept;

	private:
		uint8_t m_buffer[sizeof(T)];
	};
	template <typename T>
	template <typename ... ARGS>
	void Deferred<T>::create(const ARGS & ... args) noexcept
	{
		new(((T*)(void*)this)) T(args ...);
	}
	template <typename T>
	void Deferred<T>::remove() noexcept
	{
		((T*)(void*)this)->~T();
	}
	template <typename T>
	Deferred<T>::operator T*() noexcept
	{
		return (T*)(void*)this;
	}
	template <typename T>
	T* Deferred<T>::operator ->() noexcept
	{
		return (T*)(void*)this;
	}
	
	template <typename T>
	class Pointer
	{
	protected:
		T * m_ptr;

	public:
		Pointer() = default;
		Pointer(T * ptr) noexcept
			:m_ptr(ptr)
		{
		}
		Pointer<T>& operator =(T* ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}
		T * operator ->() const noexcept
		{
			return m_ptr;
		}
		operator T*() const noexcept
		{
			return m_ptr;
		}

		Pointer<Pointer<T>> operator &() noexcept
		{
			return this;
		}
		Pointer<const Pointer<T>> operator &() const noexcept
		{
			return this;
		}
	};

	template <typename T>
	class Pointer<Pointer<T>>
	{
	protected:
		T ** m_ptr;

	public:
		Pointer() = default;
		Pointer(T ** ptr) noexcept
			:m_ptr(ptr)
		{
		}
		Pointer(Pointer<T> * ptr) noexcept
			:m_ptr((T**)ptr)
		{
		}
		Pointer<T>& operator =(T** ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}
		Pointer<T>& operator =(Pointer<T>* ptr) noexcept
		{
			m_ptr = (T**)ptr;
			return *this;
		}
		operator T**() const noexcept
		{
			return m_ptr;
		}
		operator Pointer<T>*() const noexcept
		{
			return (Pointer<T>*)m_ptr;
		}
	};

	template <typename T>
	class Pointer<const Pointer<T>>
	{
	protected:
		T * const * m_ptr;

	public:
		Pointer() = default;
		Pointer(T * const * ptr) noexcept
			:m_ptr(ptr)
		{
		}
		Pointer(const Pointer<T> * ptr) noexcept
			:m_ptr((T* const *)ptr)
		{
		}
		Pointer<T>& operator =(T* const * ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}
		Pointer<T>& operator =(const Pointer<T>* ptr) noexcept
		{
			m_ptr = (T* const *)ptr;
			return *this;
		}
		operator T* const *() const noexcept
		{
			return m_ptr;
		}
		operator const Pointer<T>*() const noexcept
		{
			return (const Pointer<T>*)m_ptr;
		}
	};

	/*
	T를 자동 소멸시키는 클래스.
	참조 횟수를 가진 클래스면 생성/소멸시, 추가/감소된다.
	참조 횟수가 없다면, 소멸시 T를 delete 시킨다.
	내부 참조가 NULL일 수 없다.
	*/
	template <typename T> class Must
	{
		template <typename> friend class Must;
	public:
		Must(T* ref) noexcept;
		Must(const Must& copy) noexcept;
		template <typename T2> Must(Keep<T2>&& move) noexcept;
		template <typename T2> Must(const Must<T2>& copy) noexcept;
		~Must() noexcept;

		Must& operator =(const Must & copy) noexcept;
		template <typename T2> Must& operator =(T2* ref) noexcept;
		template <typename T2> Must& operator =(Keep<T2>&& move) noexcept;
		template <typename T2> Must& operator =(const Must<T2> & copy) noexcept;
		T* operator ->() const noexcept;
		operator T*() const noexcept;
		T& operator *() const noexcept;

	protected:
		T* m_ptr;
	};

	template <class T> class Keep;

	template <class T>
	class KeepPointer
	{
	public:
		KeepPointer(Keep<T> * ptr) noexcept
		{
			m_ptr = ptr;
		}
		Keep<T>& operator *() const noexcept
		{
			return *m_ptr;
		}
		Keep<T>** operator &() noexcept
		{
			return &m_ptr;
		}
		Keep<T>* const * operator &() const noexcept
		{
			return &m_ptr;
		}
		Keep<T>* operator ->() const noexcept
		{
			return m_ptr;
		}
		KeepPointer& operator =(Keep<T> * ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}
		operator Keep<T>*() const noexcept
		{
			return m_ptr;
		}
		operator T**() const noexcept
		{
			return &m_ptr->m_ptr;
		}
		operator void **() const noexcept
		{
			return (void**)&m_ptr->m_ptr;
		}
		bool operator !=(const KeepPointer & ptr) const noexcept
		{
			return m_ptr != ptr.m_ptr;
		}
		bool operator ==(const KeepPointer & ptr) const noexcept
		{
			return m_ptr == ptr.m_ptr;
		}
		bool operator !=(nullptr_t ptr) const noexcept
		{
			return m_ptr != ptr;
		}
		bool operator ==(nullptr_t ptr) const noexcept
		{
			return m_ptr == ptr;
		}

	protected:
		Keep<T> * m_ptr;
	};

	template <class T>
	class KeepConstPointer
	{
	public:
		KeepConstPointer(const Keep<T> * ptr) noexcept
		{
			m_ptr = ptr;
		}
		const Keep<T>& operator *() const noexcept
		{
			return *m_ptr;
		}
		const Keep<T>** operator &() noexcept
		{
			return &m_ptr;
		}
		const Keep<T>* const * operator &() const noexcept
		{
			return &m_ptr;
		}
		const Keep<T>* operator ->() const noexcept
		{
			return m_ptr;
		}
		KeepConstPointer& operator =(const Keep<T> * ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}
		operator const Keep<T>*() const noexcept
		{
			return m_ptr;
		}
		operator T*const*() const noexcept
		{
			return &m_ptr->m_ptr;
		}
		bool operator !=(const KeepConstPointer & ptr) const noexcept
		{
			return m_ptr != ptr.m_ptr;
		}
		bool operator ==(const KeepConstPointer & ptr) const noexcept
		{
			return m_ptr == ptr.m_ptr;
		}
		bool operator !=(nullptr_t ptr) const noexcept
		{
			return m_ptr != ptr;
		}
		bool operator ==(nullptr_t ptr) const noexcept
		{
			return m_ptr == ptr;
		}

	protected:
		const Keep<T> * m_ptr;
	};

	/*
	T를 자동 소멸시키는 클래스.
	참조 횟수를 가진 클래스면 생성/소멸시, 추가/감소된다.
	참조 횟수가 없다면, 소멸시 T를 delete 시킨다.
	*/
	template <class T> class Keep
	{
		template <typename> friend class Keep;
		template <typename> friend class Must;
		template <typename> friend class KeepPointer;
		template <typename> friend class KeepConstPointer;
	public:
		Keep() noexcept;
		Keep(T* ptr) noexcept;
		Keep(const Keep<T>& copy) noexcept;
		Keep(Keep<T>&& move) noexcept;
		template <typename T2> Keep(const Keep<T2>& copy) noexcept;
		template <typename T2> Keep(Keep<T2>&& move) noexcept;
		~Keep() noexcept;
		void move(T * v) noexcept;
		void remove() noexcept;
		operator T*&() noexcept;
		operator T* const&() const noexcept;
		T* operator ->() const noexcept;
		T& operator *() const noexcept
		{
			return *m_ptr;
		}
		const KeepPointer<T> operator &() noexcept;
		const KeepConstPointer<T> operator &() const noexcept;
		T* detach() noexcept;
		Keep<T>& operator =(const Keep<T> & copy) noexcept;
		Keep<T>& operator =(Keep<T> && move) noexcept;
		template <typename T2> Keep<T>& operator =(const Keep<T2> & copy) noexcept;
		template <typename T2> Keep<T>& operator =(Keep<T2> && move) noexcept;
		Keep<T>& operator =(T* ptr) noexcept;
		Keep<T>& operator =(decltype(nullptr)) noexcept;

	protected:
		T* m_ptr;
	};


	template <typename T>
	class Interface: public T
	{
	public:
		void AddRef() noexcept = delete;
		int Release() noexcept = delete;
	};

	template <typename T, typename Parent = Empty>
	class Referencable : public Interface<Parent>
	{
	public:
		Referencable() noexcept;
		void AddRef() noexcept;
		int Release() noexcept;
		int getReferenceCount() noexcept;

	private:
		int m_ref;
	};

	template <typename T, typename Parent = Empty>
	class AtomicReferencable : public Interface<Parent>
	{
	public:
		AtomicReferencable() noexcept;
		void AddRef() noexcept;
		int Release() noexcept;
		int getReferenceCount() noexcept;

	protected:
		std::atomic<int> m_ref;
	};

	template <typename T> 
	class Counter :public Referencable<T,T>
	{
	public:
		using Referencable<T,T>::Referencable;
	};

	class VCounter: public Referencable<VCounter>
	{
	public:
		VCounter() noexcept;
		virtual ~VCounter() noexcept;
	};

	template <typename T> class StateKeeper
	{
	public:
		template <typename ... ARGS> StateKeeper(ARGS && ... values)
		{
			m_prev = T(move(values) ...).use();
		}
		~StateKeeper()
		{
			m_prev.use();
		}
	private:
		T m_prev;
	};

	// 템플릿 파라미터로 주는 타입을 동적 할당으로 관리한다.
	// Pimpl을 위한 클래스
	template <typename T> class Dynamic
	{
	public:
		Dynamic();
		Dynamic(Dynamic&& _move);
		~Dynamic();
		T* operator ->();
		operator T*();

	protected:
		Dynamic(const T&);
		T* m_ptr;
	};

	template<class T>
	inline Manual<T>::Manual() noexcept
	{
		ondebug(m_constructed = false);
	}
	template<class T>
	template <typename ...ARGS>
	inline void Manual<T>::create(const ARGS & ...args)
	{
		ondebug(_assert(!m_constructed));
		ondebug(m_constructed = true);
		new(m_buffer) T(move(args) ...);
	}
	template<class T>
	inline void Manual<T>::remove() noexcept
	{
		ondebug(_assert(m_constructed));
		ondebug(m_constructed = false);
		((T*)m_buffer)->~T();
	}
	template<class T>
	inline T* Manual<T>::address() noexcept
	{
		return (T*)m_buffer;
	}
	template<class T>
	inline T* Manual<T>::operator ->() noexcept
	{
		ondebug(_assert(m_constructed));
		return (T*)m_buffer;
	}
	template<class T>
	inline Manual<T>::operator T*() noexcept
	{
		ondebug(_assert(m_constructed));
		return (T*)m_buffer;
	}
	template<class T>
	inline const T* Manual<T>::operator ->() const noexcept
	{
		ondebug(_assert(m_constructed));
		return (const T*)m_buffer;
	}
	template<class T>
	inline Manual<T>::operator const T*() const noexcept
	{
		ondebug(_assert(m_constructed));
		return (const T*)m_buffer;
	}

	template <typename T>
	inline Must<T>::Must(T* ref) noexcept :m_ptr(ref)
	{
		_assert(ref != nullptr);
		Releaser::addRefFirst(ref);
	}
	template <typename T>
	inline Must<T>::Must(const Must& copy) noexcept
	{
		m_ptr = copy.m_ptr;
		Releaser::addRef(m_ptr);
	}
	template <typename T>
	template <typename T2>
	inline Must<T>::Must(Keep<T2>&& move) noexcept
	{
		m_ptr = move.m_ptr;
		move.m_ptr = nullptr;
	}
	template <typename T>
	template <typename T2> 
	inline Must<T>::Must(const Must<T2>& copy) noexcept
	{
		m_ptr = copy.m_ptr;
		Releaser::addRef(m_ptr);
	}
	template <typename T>
	inline Must<T>::~Must() noexcept
	{
		Releaser::release(m_ptr);
	}
	template <typename T>
	inline Must<T>& Must<T>::operator =(const Must & copy) noexcept
	{
		this->~Must();
		m_ptr = copy.m_ptr;
		Releaser::addRef(m_ptr);
		return *this;
	}
	template <typename T>
	template <typename T2> 
	inline Must<T>& Must<T>::operator =(T2* ref) noexcept
	{
		_assert(ref != nullptr);
		this->~Must();
		m_ptr = ref;
		Releaser::addRef(ref);
		return *this;
	}
	template <typename T>
	template <typename T2>
	inline Must<T>& Must<T>::operator =(Keep<T2> && move) noexcept
	{
		this->~Must();
		m_ptr = move.m_ptr;
		move.m_ptr = nullptr;
		return *this;
	}
	template <typename T>
	template <typename T2> 
	inline Must<T>& Must<T>::operator =(const Must<T2> & copy) noexcept
	{
		this->~Must();
		m_ptr = copy.m_ptr;
		Releaser::addRef(m_ptr);
		return *this;
	}
	template <typename T>
	inline T* Must<T>::operator ->() const noexcept
	{
		return m_ptr;
	}
	template <typename T>
	inline Must<T>::operator T*() const noexcept
	{
		return m_ptr;
	}
	template <typename T>
	inline T& Must<T>::operator *() const noexcept
	{
		return *m_ptr;
	}

	template<class T>
	inline Keep<T>::Keep() noexcept
	{
		m_ptr = nullptr;
	}
	template<class T>
	inline Keep<T>::Keep(T * ptr) noexcept
		: m_ptr(ptr)
	{
		if (ptr != nullptr) Releaser::addRefFirst(ptr);
	}
	template<class T>
	inline Keep<T>::Keep(const Keep<T>& copy) noexcept
	{
		m_ptr = copy.m_ptr;
		if (m_ptr != nullptr) Releaser::addRef(m_ptr);
	}
	template<class T>
	inline Keep<T>::Keep(Keep<T>&& move) noexcept
	{
		m_ptr = move.m_ptr;
		move.m_ptr = nullptr;
	}
	template<class T>
	template <typename T2>
	inline Keep<T>::Keep(const Keep<T2>& copy) noexcept
	{
		m_ptr = copy.m_ptr;
		if (m_ptr != nullptr) Releaser::addRef(m_ptr);
	}
	template<class T>
	template <typename T2>
	inline Keep<T>::Keep(Keep<T2>&& move) noexcept
	{
		m_ptr = move.m_ptr;
		move.m_ptr = nullptr;
	}
	template<class T>
	inline Keep<T>::~Keep() noexcept
	{
		if (m_ptr != nullptr) Releaser::release(m_ptr);
	}
	template<class T>
	inline void Keep<T>::move(T * v) noexcept
	{
		this->~Keep();
		m_ptr = v;
	}
	template<class T>
	inline void Keep<T>::remove() noexcept
	{
		if (m_ptr != nullptr)
		{
			Releaser::release(m_ptr);
			m_ptr = nullptr;
		}
	}
	template<class T>
	inline Keep<T>::operator T*&() noexcept
	{
		return m_ptr;
	}
	template<class T>
	inline Keep<T>::operator T* const&() const noexcept
	{
		return m_ptr;
	}
	template<class T>
	inline T* Keep<T>::operator ->() const noexcept
	{
		_assert(m_ptr != nullptr);
		return m_ptr;
	}
	template<class T>
	inline const KeepPointer<T> Keep<T>::operator &() noexcept
	{
		return this;
	}
	template<class T>
	inline const KeepConstPointer<T> Keep<T>::operator &() const noexcept
	{
		return this;
	}
	template<class T>
	inline T* Keep<T>::detach() noexcept
	{
		T * p = m_ptr;
		m_ptr = nullptr;
		return p;
	}
	template<class T>
	Keep<T>& Keep<T>::operator =(const Keep<T> & copy) noexcept
	{
		this->~Keep();
		m_ptr = copy.m_ptr;
		if (m_ptr != nullptr) Releaser::addRef(m_ptr);
		return *this;
	}
	template<class T>
	Keep<T>& Keep<T>::operator =(Keep<T> && move) noexcept
	{
		this->~Keep();
		m_ptr = move.m_ptr;
		move.m_ptr = nullptr;
		return *this;
	}
	template<class T>
	template <typename T2>
	inline Keep<T>& Keep<T>::operator =(const Keep<T2> & copy) noexcept
	{
		this->~Keep();
		m_ptr = copy.m_ptr;
		if (m_ptr != nullptr) Releaser::addRef(m_ptr);
		return *this;
	}
	template<class T>
	template <typename T2>
	inline Keep<T>& Keep<T>::operator =(Keep<T2> && move) noexcept
	{
		this->~Keep();
		m_ptr = move.m_ptr;
		move.m_ptr = nullptr;
		return *this;
	}
	template<class T>
	inline Keep<T>& Keep<T>::operator =(T* ptr) noexcept
	{
		this->~Keep();
		new(this) Keep<T>(ptr);
		return *this;
	}
	template<class T>
	inline Keep<T>& Keep<T>::operator =(decltype(nullptr)) noexcept
	{
		if (m_ptr != nullptr)
		{
			Releaser::release(m_ptr);
			m_ptr = nullptr;
		}
		return *this;
	}
	
	template<typename T>
	inline Dynamic<T>::Dynamic()
	{
		if (alignof(T) != 1)
		{
			m_ptr = _newAligned(T);
		}
		else
		{
			m_ptr = _new T;
		}
	}
	template<typename T>
	Dynamic<T>::Dynamic(Dynamic&& _move)
	{
		m_ptr = _move.m_ptr;
		_move.m_ptr = nullptr;
	}
	template<typename T>
	inline Dynamic<T>::~Dynamic()
	{
		if (alignof(T) != 1)
		{
			_deleteAligned(m_ptr);
		}
		else
		{
			delete m_ptr;
		}
	}
	template<typename T>
	inline T* Dynamic<T>::operator ->()
	{
		return m_ptr;
	}
	template<typename T>
	inline Dynamic<T>::operator T*()
	{
		return m_ptr;
	}

	template<typename T, typename Parent>
	Referencable<T, Parent>::Referencable() noexcept
	{
		m_ref = 0;
	}
	template<typename T, typename Parent>
	void Referencable<T, Parent>::AddRef() noexcept
	{
		m_ref++;
	}
	template<typename T, typename Parent>
	int Referencable<T, Parent>::Release() noexcept
	{
		_assert(m_ref != 0);
		m_ref--;
		int ref = m_ref;
		if (ref == 0) 
			delete static_cast<T*>(this);
		return ref;
	}
	template<typename T, typename Parent>
	inline int Referencable<T, Parent>::getReferenceCount() noexcept
	{
		return m_ref;
	}

	template<typename T, typename Parent>
	AtomicReferencable<T, Parent>::AtomicReferencable() noexcept
		:m_ref(0)
	{
	}
	template<typename T, typename Parent>
	void AtomicReferencable<T, Parent>::AddRef() noexcept
	{
		m_ref++;
	}
	template<typename T, typename Parent>
	int AtomicReferencable<T, Parent>::Release() noexcept
	{
		int old = m_ref--;
		_assert(old != 0);
		old--;
		if (old == 0) delete static_cast<T*>(this);
		return old;
	}
	template<typename T, typename Parent>
	inline int AtomicReferencable<T, Parent>::getReferenceCount() noexcept
	{
		return m_ref;
	}
}