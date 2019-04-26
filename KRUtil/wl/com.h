#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/main.h>
#include <KR3/wl/windows.h>

#include <Unknwn.h>
#include <kr3/meta/types.h>

#define COM_CLASS_DECLARE(name) template <> struct ComMethod<name>;
#define COM_CLASS_METHODS(name) template <> struct ComMethod<name> : ComMethodBasic<name>
#define COM_INHERIT(name, parent) template <> struct ComParent<name> { using type = parent; };

namespace kr
{

#define OVERRIDE_UNKNOWN() \
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv) override{ return ComImplement::QueryInterface(riid,ppv); };\
	virtual ULONG STDMETHODCALLTYPE AddRef() override{ return ComImplement::AddRef(); };\
	virtual ULONG STDMETHODCALLTYPE Release() override{ return ComImplement::Release(); };

	template <typename T>
	HRESULT createCOMObject(T ** p, REFCLSID rclsid, LPUNKNOWN pUnkOuter = nullptr, DWORD dwClsContext = CLSCTX_ALL)
	{
		return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)p);
	}

	template <class T, class ... ARGS> class ComImplement:public ARGS...
	{
	protected:
		IUnknown* _GetUnknown()
		{
			return static_cast<IUnknown*>(this);
		}

		dword m_nReference;

		template <typename T> int _queryInterface(REFIID riid, void ** ppv)
		{
			if (__uuidof(T) == riid)
			{
				*ppv = static_cast<T*>(this);
				AddRef();
				throw (int)0;
			}
			return 0;
		}

	public:
		ComImplement()
		{
			m_nReference = 1;
		}
		
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv) override
		{
			if(ppv == nullptr) return E_POINTER;
			try
			{
				unpackR(_queryInterface<ARGS>(riid, ppv));
				return E_NOINTERFACE;
			}
			catch(int)
			{
				return NOERROR;
			}
		}
		virtual ULONG STDMETHODCALLTYPE AddRef() override
		{
			return InterlockedIncrement(&m_nReference);
		}
		virtual ULONG STDMETHODCALLTYPE Release() override
		{
			UINT nRef = InterlockedDecrement(&m_nReference);
			if(nRef == 0) delete static_cast<T*>(this);
			return nRef;
		}
	};

	class ComData;
	template <typename T>
	class ComPointer;
	template <typename T>
	struct ComMethod;
	template <typename T>
	class Com;

	template <typename T>
	struct ComParent
	{
		using type = IUnknown;
	};
	template <>
	struct ComParent<IUnknown>
	{
		using type = void;
	};

	template <typename T>
	using com_parent_t = typename ComParent<T>::type;

	template <typename BASE, typename T, bool check = false>
	struct com_is_base_of:
		com_is_base_of<BASE, com_parent_t<T>, is_same<T, BASE>::value>
	{
	};
	template <typename BASE, typename T>
	struct com_is_base_of<BASE, T, true> : meta::bool_true {};

	template <typename BASE>
	struct com_is_base_of<BASE, void, false> : meta::bool_false {};

	class ComData
	{
	protected:
		IUnknown * m_ptr;

	public:
		ComData() noexcept;
		void remove() noexcept;

		template <typename T2>
		Com<T2> as() const noexcept;
	};

	template <typename T>
	class ComMethodBasic: public ComMethod<com_parent_t<T>>
	{
	public:
		T * & ptr() noexcept;
		T * const & ptr() const noexcept;
	};

	template <typename T>
	struct ComMethod : ComMethodBasic<T>
	{
	};
	
	template <>
	struct ComMethod<void>: ComData
	{
	public:
	};

	template <typename T>
	class ComPointer
	{
	private:
		Com<T> * m_pptr;

	public:
		ComPointer(Com<T> * pptr) noexcept
			:m_pptr(pptr)
		{
		}
		operator Com<T>*() const
		{
			return m_pptr;
		}
		operator T**() const noexcept
		{
			return (T**)&m_pptr->m_ptr;
		}
		operator void**() const noexcept
		{
			return (void**)&m_pptr->m_ptr;
		}
		operator IUnknown**() const noexcept
		{
			return (IUnknown**)&m_pptr->m_ptr;
		}
	};

	template <typename T>
	class Com : public ComMethod<T>
	{
		template <typename T>
		friend class Com;
		template <typename T>
		friend class ComPointer;
		using ParentT = com_parent_t<T>;

	protected:
		using ComData::m_ptr;

	public:
		using ComMethod<T>::ComMethod;
		using ComData::remove;
		using ComMethodBasic<T>::ptr;

		static const Com<T>& wrap(const T * &ptr) noexcept
		{
			return &ptr;
		}

		const ComPointer<T> operator &() noexcept
		{
			return this;
		}

		Com() noexcept
		{
			m_ptr = nullptr;
		}
		~Com() noexcept
		{
			if (m_ptr) ((IUnknown*)m_ptr)->Release();
		}
		Com(nullptr_t) noexcept
		{
			m_ptr = nullptr;
		}
		Com(T * ptr) noexcept
		{
			m_ptr = (IUnknown*)ptr;
			if (ptr) ((IUnknown*)ptr)->AddRef();
		}
		Com(const Com & _copy) noexcept
			:Com((T*)_copy.m_ptr)
		{
		}
		Com(Com && _move) noexcept
		{
			m_ptr = _move.m_ptr;
			_move.m_ptr = nullptr;
		}
		template <typename T2>
		Com(const Com<T2> & _copy) noexcept
			:Com((T*)_copy.m_ptr)
		{
			static_assert(com_is_base_of<T, T2>::value, "Cannot cast");
		}
		template <typename T2>
		Com(Com<T2> && _move) noexcept
		{
			static_assert(com_is_base_of<T, T2>::value, "Cannot cast");

			m_ptr = _move.m_ptr;
			_move.m_ptr = nullptr;
		}
		Com & operator = (T * ptr) noexcept
		{
			remove();
			m_ptr = (IUnknown*)ptr;
			if (ptr) ((IUnknown*)ptr)->AddRef();
			return *this;
		}
		Com & operator = (nullptr_t) noexcept
		{
			remove();
			m_ptr = nullptr;
			return *this;
		}
		template <typename T2>
		Com & operator = (const Com<T2> & _copy) noexcept
		{
			static_assert(com_is_base_of<T, T2>::value, "Cannot cast");

			remove();
			IUnknown * ptr = _copy.m_ptr;
			m_ptr = ptr;
			if (ptr) ptr->AddRef();
			return *this;
		}
		template <typename T2>
		Com & operator = (Com<T2> && _move) noexcept
		{
			static_assert(com_is_base_of<T, T2>::value, "Cannot cast");

			remove();
			m_ptr = _move.m_ptr;
			_move.m_ptr = nullptr;
			return *this;
		}
		Com & operator = (const Com & _copy) noexcept
		{
			remove();
			IUnknown * ptr = _copy.m_ptr;
			m_ptr = ptr;
			if (ptr) ptr->AddRef();
			return *this;
		}
		Com & operator = (Com && _move) noexcept
		{
			remove();
			m_ptr = _move.m_ptr;
			_move.m_ptr = nullptr;
			return *this;
		}
		T * operator ->() const noexcept
		{
			return (T*)m_ptr;
		}
		operator T*() const noexcept
		{
			return (T*)m_ptr;
		}
		operator ParentT*() const noexcept
		{
			return (ParentT*)m_ptr;
		}

		bool operator != (nullptr_t) const noexcept
		{
			return m_ptr != nullptr;
		}
		bool operator == (nullptr_t) const noexcept
		{
			return m_ptr == nullptr;
		}
		template <typename T2>
		bool operator != (const Com<T2> & other) const noexcept
		{
			return m_ptr != other.m_ptr;
		}
		template <typename T2>
		bool operator == (const Com<T2> & other) const noexcept
		{
			return m_ptr == other.m_ptr;
		}
		bool operator != (T * other) const noexcept
		{
			return m_ptr != other;
		}
		bool operator == (T * other) const noexcept
		{
			return m_ptr == other;
		}
		bool operator != (ParentT * other) const noexcept
		{
			return m_ptr != other;
		}
		bool operator == (ParentT * other) const noexcept
		{
			return m_ptr == other;
		}
	};

	template <typename T2>
	Com<T2> ComData::as() const noexcept
	{
		Com<T2> ptr;
		if (FAILED(((IUnknown*)m_ptr)->QueryInterface(__uuidof(T2), &ptr))) return nullptr;
		return ptr;
	}
	template <typename T>
	T *& ComMethodBasic<T>::ptr() noexcept {
		return (T*&)m_ptr;
	}
	template <typename T>
	T * const & ComMethodBasic<T>::ptr() const noexcept {
		return (T*&)m_ptr;
	}
}

#ifdef NDEBUG
#define hrmustbe(x) {HRESULT __hr; if(FAILED(__hr = (x))) exit(__hr); }
#define hrshouldbe(x) (SUCCEEDED(x))
#define hrexcept(x)	{HRESULT __hr; if(FAILED(__hr = (x))) throw ::kr::ErrorCode(__hr); }
#else
#define hrmustbe(x) {\
	HRESULT hr = (x);\
	if(FAILED(hr)) { ::kr::error(#x "\r\nHRESULT: 0x%08X\r\nMessage: %s",hr, ErrorCode(hr).getMessage<char>()); }\
}
#define hrshouldbe(x) ([&]{\
	HRESULT hr = (x);\
	if(FAILED(hr)){\
		::kr::dout << #x << ::kr::endl; \
		::kr::dout << __FILE__ << '(' << __LINE__ << ')' << ::kr::endl; \
		::kr::dout << "HRESULT: 0x" << ::kr::hexf((uint32_t)hr, 8) << ::kr::endl; \
		::kr::dout << "Message: " << ErrorCode(hr).getMessage<char>() << ::kr::endl; \
		return false; \
	}\
	return true;}())

#define hrexcept(x) {\
HRESULT hr = (x);\
 if(FAILED(hr)) {\
	::kr::dout << #x << ::kr::endl; \
	::kr::dout << __FILE__ << '(' << __LINE__ << ')' << ::kr::endl; \
	::kr::dout << "HRESULT: 0x" << ::kr::hexf((uint32_t)hr, 8) << ::kr::endl; \
	throw ::kr::ErrorCode(hr);\
 } }
#endif