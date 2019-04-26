#pragma once


template <typename T>
class ComPtr
{
public:
	typedef T InterfaceType;

protected:
	InterfaceType *ptr_;
	template<class U> friend class ComPtr;

	void InternalAddRef() const throw()
	{
		if (ptr_ != nullptr)
		{
			ptr_->AddRef();
		}
	}

	unsigned long InternalRelease() throw()
	{
		unsigned long ref = 0;
		T* temp = ptr_;

		if (temp != nullptr)
		{
			ptr_ = nullptr;
			ref = temp->Release();
		}

		return ref;
	}

public:
#pragma region constructors
	ComPtr() throw() : ptr_(nullptr)
	{
	}

	ComPtr(decltype(__nullptr)) throw() : ptr_(nullptr)
	{
	}

	template<class U>
	ComPtr(_In_opt_ U *other) throw() : ptr_(other)
	{
		InternalAddRef();
	}

	ComPtr(const ComPtr& other) throw() : ptr_(other.ptr_)
	{
		InternalAddRef();
	}

	// copy ctor that allows to instanatiate class when U* is convertible to T*
	template<class U>
	ComPtr(const ComPtr<U> &other) throw() :
		ptr_(other.ptr_)
	{
		InternalAddRef();
	}

	ComPtr(_Inout_ ComPtr &&other) throw() : ptr_(nullptr)
	{
		if (this != reinterpret_cast<ComPtr*>(&reinterpret_cast<unsigned char&>(other)))
		{
			Swap(other);
		}
	}

	// Move ctor that allows instantiation of a class when U* is convertible to T*
	template<class U>
	ComPtr(_Inout_ ComPtr<U>&& other) throw() :
		ptr_(other.ptr_)
	{
		other.ptr_ = nullptr;
	}
#pragma endregion

#pragma region destructor
	~ComPtr() throw()
	{
		InternalRelease();
	}
#pragma endregion

#pragma region assignment
	ComPtr& operator=(decltype(__nullptr)) throw()
	{
		InternalRelease();
		return *this;
	}

	ComPtr& operator=(_In_opt_ T *other) throw()
	{
		if (ptr_ != other)
		{
			ComPtr(other).Swap(*this);
		}
		return *this;
	}

	template <typename U>
	ComPtr& operator=(_In_opt_ U *other) throw()
	{
		ComPtr(other).Swap(*this);
		return *this;
	}

	ComPtr& operator=(const ComPtr &other) throw()
	{
		if (ptr_ != other.ptr_)
		{
			ComPtr(other).Swap(*this);
		}
		return *this;
	}

	template<class U>
	ComPtr& operator=(const ComPtr<U>& other) throw()
	{
		ComPtr(other).Swap(*this);
		return *this;
	}

	ComPtr& operator=(_Inout_ ComPtr &&other) throw()
	{
		ComPtr(static_cast<ComPtr&&>(other)).Swap(*this);
		return *this;
	}

	template<class U>
	ComPtr& operator=(_Inout_ ComPtr<U>&& other) throw()
	{
		ComPtr(static_cast<ComPtr<U>&&>(other)).Swap(*this);
		return *this;
	}
#pragma endregion

#pragma region modifiers
	void Swap(_Inout_ ComPtr&& r) throw()
	{
		T* tmp = ptr_;
		ptr_ = r.ptr_;
		r.ptr_ = tmp;
	}

	void Swap(_Inout_ ComPtr& r) throw()
	{
		T* tmp = ptr_;
		ptr_ = r.ptr_;
		r.ptr_ = tmp;
	}
#pragma endregion

	T* Get() const throw()
	{
		return ptr_;
	}

	typename InterfaceType* operator->() const throw()
	{
		return ptr_;
	}

	InterfaceType** operator&() throw()
	{
		return &ptr_;
	}

	InterfaceType* const * operator&() const throw()
	{
		return &ptr_;
	}

	T* const* GetAddressOf() const throw()
	{
		return &ptr_;
	}

	T** GetAddressOf() throw()
	{
		return &ptr_;
	}

	T** ReleaseAndGetAddressOf() throw()
	{
		InternalRelease();
		return &ptr_;
	}

	T* Detach() throw()
	{
		T* ptr = ptr_;
		ptr_ = nullptr;
		return ptr;
	}

	void Attach(_In_opt_ InterfaceType* other) throw()
	{
		if (ptr_ != nullptr)
		{
			auto ref = ptr_->Release();
			(ref);
			// Attaching to the same object only works if duplicate references are being coalesced. Otherwise
			// re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
			__WRL_ASSERT__(ref != 0 || ptr_ != other);
		}

		ptr_ = other;
	}

	unsigned long Reset()
	{
		return InternalRelease();
	}

	HRESULT CopyTo(InterfaceType** ptr) const throw()
	{
		InternalAddRef();
		*ptr = ptr_;
		return S_OK;
	}

	HRESULT CopyTo(REFIID riid, void** ptr) const throw()
	{
		return ptr_->QueryInterface(riid, ptr);
	}

	template<typename U>
	HRESULT CopyTo(U** ptr) const throw()
	{
		return ptr_->QueryInterface(__uuidof(U), reinterpret_cast<void**>(ptr));
	}

	// query for U interface
	template<typename U>
	HRESULT As(_Inout_ U** p) const throw()
	{
		return ptr_->QueryInterface(__uuidof(U), (void**)p);
	}

	// query for U interface
	template<typename U>
	HRESULT As(_Out_ ComPtr<U>* p) const throw()
	{
		return ptr_->QueryInterface(__uuidof(U), reinterpret_cast<void**>(p->ReleaseAndGetAddressOf()));
	}

	// query for riid interface and return as IUnknown
	HRESULT AsIID(REFIID riid, _Out_ ComPtr<IUnknown>* p) const throw()
	{
		return ptr_->QueryInterface(riid, reinterpret_cast<void**>(p->ReleaseAndGetAddressOf()));
	}
};    // ComPtr