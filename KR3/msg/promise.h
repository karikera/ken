#pragma once

#include <KR3/main.h>
#include <KR3/meta/function.h>

namespace kr
{
	class EventPump;

	template <typename T>
	class Promise;
	template <typename T>
	class PromisePass;
	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseThen;
	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseKatch;
	using PromiseVoid = Promise<void>;
	template <typename T>
	class DeferredPromise;

	template <typename T>
	struct unwrap_promise
	{
		using type = T;
	};
	template <typename T>
	struct unwrap_promise<Promise<T>*>
	{
		using type = T;
	};
	template <>
	struct unwrap_promise<void>
	{
		using type = void;
	};
	template <typename T>
	using unwrap_promise_t = typename unwrap_promise<T>::type;

	class PromiseRaw;

	namespace promise_meta
	{
		template <typename In, typename Out>
		struct with
		{
			template <typename LAMBDA>
			static void call(Out* dest, LAMBDA&& lambda, In* value)
			{
				new(dest) Out(lambda(*value));
			}
			static void substitution(Out* dest, In* value)
			{
				new(dest) Out(move(*value));
			}
		};
		template <typename Out>
		struct with<void, Out>
		{
			template <typename LAMBDA>
			static void call(Out* dest, LAMBDA&& lambda, void*)
			{
				new(dest) Out(lambda());
			}
			//static void substitution(Out * dest, void *)
			//{
			//	new(dest) Out();
			//}
		};
		template <typename In>
		struct with<In, void>
		{
			template <typename LAMBDA>
			static void call(void*, LAMBDA&& lambda, In* value)
			{
				lambda(*value);
			}
			//static void substitution(void * dest, In * value)
			//{
			//}
		};
		template <>
		struct with<void, void>
		{
			template <typename LAMBDA>
			static void call(void*, LAMBDA&& lambda, void*)
			{
				lambda();
			}
			static void substitution(void*, void*)
			{
			}
		};
	}

	class PromiseRaw
	{
		friend EventPump;

		template <typename T>
		friend class Promise;
		template <typename InType, typename OutType, typename LAMBDA>
		friend class PromiseThen;
		template <typename InType, typename OutType, typename LAMBDA>
		friend class PromiseKatch;
		template <typename T>
		friend class DeferredPromise;
	public:
		enum State
		{
			Pending,
			Resolved,
			Rejected,
		};

		PromiseRaw() noexcept;
		virtual ~PromiseRaw() noexcept;

		template <typename ... ARGS>
		static Promise<void>* all(Promise<ARGS>* ... proms) noexcept;
	protected:
		std::exception_ptr* _rejectValue() noexcept;
		void _rejectException(std::exception_ptr data) noexcept;

		// same with rejectException(std::current_exception())
		void _reject() noexcept;
		template <typename REJTYPE>
		void _reject(REJTYPE rej) noexcept;

		void _resolveCommit() noexcept;
		void _rejectCommit() noexcept;
		virtual void onThen(PromiseRaw * from) noexcept;
		virtual void onKatch(PromiseRaw * from) noexcept;

	private:
		ATTR_DEPRECATED("all tasks must complete") void _deleteCascade() noexcept;
		void _setState(State state) noexcept;
		void _addNext(PromiseRaw * next) noexcept;
		void _readdNext(PromiseRaw * next) noexcept;

		State m_state;
		bool m_ending;
		PromiseRaw * m_next;
		PromiseRaw * m_sibling;
		PromiseRaw ** m_psibling;
	};

	template <typename T>
	class Promise : public PromiseRaw
	{
		template <typename InType, typename OutType, typename LAMBDA>
		friend class PromiseThen;
		template <typename InType, typename OutType, typename LAMBDA>
		friend class PromiseKatch;
		template <typename T2>
		friend class PromisePass;
		template <typename T2>
		friend class DeferredPromise;
	public:
		using ResultType = T;
		virtual ~Promise() noexcept override;
		template <typename LAMBDA>
		auto then(LAMBDA &&lambda) noexcept->Promise<unwrap_promise_t<decltype(lambda((T&)*(T*)0))> >*;
		template <typename LAMBDA>
		auto katch(LAMBDA &&lambda) noexcept->Promise<unwrap_promise_t<decltype(lambda(nullref))> >*;
		void connect(DeferredPromise<T>* prom) noexcept;
		static Promise<T>* resolve(T &&data) noexcept;
		static Promise<T>* resolve(const T &data) noexcept;
		static Promise<T> * rejectException(std::exception_ptr data) noexcept;
		static Promise<T> * reject() noexcept;
		template <typename REJTYPE>
		static Promise<T> * reject(REJTYPE rej) noexcept;
		static Promise<void>* all(View<Promise<T>*> proms) noexcept;

	protected:
		void _resolve(const T & data) noexcept;
		void _resolve(T && data) noexcept;

		T * _resolveValue() noexcept;

	private:
		alignas(meta::maxt(alignof(T), alignof(std::exception_ptr))) struct {
			byte buffer[meta::maxt(sizeof(T), sizeof(std::exception_ptr))];
		} m_data;
	};

	template <>
	class Promise<void> : public PromiseRaw
	{
		template <typename InType, typename OutType, typename LAMBDA>
		friend class PromiseThen;
		template <typename InType, typename OutType, typename LAMBDA>
		friend class PromiseKatch;
		template <typename T>
		friend class PromisePass;
		template <typename T>
		friend class DeferredPromise;
	public:
		using ResultType = void;
		template <typename LAMBDA>
		auto then(LAMBDA &&lambda) noexcept->Promise<unwrap_promise_t<decltype(lambda())>>*;
		template <typename LAMBDA>
		auto katch(LAMBDA &&lambda) noexcept->Promise<unwrap_promise_t<decltype(lambda(nullref))>>*;
		void connect(DeferredPromise<void>* prom) noexcept;
		static PromiseVoid * resolve() noexcept;
		static PromiseVoid * rejectException(std::exception_ptr data) noexcept;
		static PromiseVoid * reject() noexcept;
		template <typename REJTYPE>
		static PromiseVoid * reject(REJTYPE rej) noexcept;
		static Promise<void>* all(View<Promise<void>*> proms) noexcept;

	protected:
		void _resolve() noexcept;

		void * _resolveValue() noexcept;

	private:

		alignas(alignof(std::exception_ptr)) struct {
			byte buffer[sizeof(std::exception_ptr)]; 
		} m_data;
	};

	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseWithLambda : public Promise<unwrap_promise_t<OutType>>
	{
	private:
		LAMBDA m_lambda;

	public:
		PromiseWithLambda(LAMBDA&& lambda) noexcept
			:m_lambda(move(lambda))
		{
		}
		PromiseWithLambda(const LAMBDA& lambda) noexcept
			:m_lambda(lambda)
		{
		}

	protected:
		void _callLambda(OutType * dest, InType * value)
		{
			return promise_meta::with<InType, OutType>::call(dest, m_lambda, value);
		}
		template <typename Data>
		void _callLambdaKatch(OutType * dest, Data *value)
		{
			return promise_meta::with<Data, OutType>::call(dest, m_lambda, value);
		}
	};

	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseThen : public PromiseWithLambda<InType, OutType, LAMBDA>
	{
		using Super = PromiseWithLambda<InType, OutType, LAMBDA>;
	protected:
		using PromiseRaw::_resolveCommit;
		using Super::_reject;
		using Super::_callLambda;
		using Super::_resolveValue;
		using Super::_rejectException;

	public:
		PromiseThen(LAMBDA&& lambda) noexcept
			:Super(move(lambda))
		{
		}
		PromiseThen(const LAMBDA& lambda) noexcept
			:Super(lambda)
		{
		}

		virtual void onThen(PromiseRaw * from) noexcept override
		{
			try
			{
				InType * data = static_cast<Promise<InType>*>(from)->_resolveValue();
				_callLambda(_resolveValue(), data);
				return _resolveCommit();
			}
			catch (...)
			{
				return _reject();
			}
		}
		virtual void onKatch(PromiseRaw * from) noexcept override
		{
			return _rejectException(*from->_rejectValue());
		}
	};

	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseKatch : public PromiseWithLambda<InType, OutType, LAMBDA>
	{
		using Super = PromiseWithLambda<InType, OutType, LAMBDA>;
	private:
		using PromiseRaw::_resolveCommit;
		using Super::_reject;
		using Super::_callLambdaKatch;
		using Super::_resolveValue;

	public:
		PromiseKatch(LAMBDA&& lambda) noexcept
			:Super(move(lambda))
		{
		}
		PromiseKatch(const LAMBDA& lambda) noexcept
			:Super(lambda)
		{
		}

		virtual void onThen(PromiseRaw * from) noexcept override
		{
			try
			{
				InType * data = static_cast<Promise<InType>*>(from)->_resolveValue();
				promise_meta::with<InType, OutType>::substitution(_resolveValue(), data);
				return _resolveCommit();
			}
			catch (...)
			{
				return _reject();
			}
		}
		virtual void onKatch(PromiseRaw * from) noexcept override
		{
			try
			{
				using ExType = meta::typeAt<typename meta::function<LAMBDA>::args_t, 0>;
				using ExTypeBasic = remove_const_t<remove_reference_t<ExType>>;
				if (is_same_v<ExTypeBasic, exception_ptr>)
				{
					exception_ptr * data = from->_rejectValue();
					_callLambdaKatch(_resolveValue(), (ExTypeBasic*)data);
					return _resolveCommit();
				}
				else
				{
					try
					{
						std::rethrow_exception(*from->_rejectValue());
					}
					catch (ExType& ex)
					{
						_callLambdaKatch(_resolveValue(), &ex);
						return _resolveCommit();
					}
				}
			}
			catch (...)
			{
				return _reject();
			}
		}
	};

	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseThen<InType, Promise<OutType>*, LAMBDA> : public PromiseWithLambda<InType, Promise<OutType>*, LAMBDA>
	{
		using Super = PromiseWithLambda<InType, Promise<OutType>*, LAMBDA>;

	private:
		using PromiseRaw::_resolveCommit;
		using Super::_resolveValue;
		using Super::_rejectException;
		using Super::_reject;
		using Super::_callLambda;
		bool m_reposted;

	public:
		PromiseThen(LAMBDA&& lambda) noexcept
			:Super(move(lambda))
		{
			m_reposted = false;
		}
		PromiseThen(const LAMBDA& lambda) noexcept
			:Super(lambda)
		{
			m_reposted = false;
		}

		virtual void onThen(PromiseRaw * from) noexcept override
		{
			if (m_reposted)
			{
				using RealOutType = unwrap_promise_t<OutType>;
				RealOutType * data = static_cast<Promise<RealOutType>*>(from)->_resolveValue();
				promise_meta::with<RealOutType, RealOutType>::substitution(_resolveValue(), data);
				return _resolveCommit();
			}
			try
			{
				InType * data = static_cast<Promise<InType>*>(from)->_resolveValue();
				Promise<OutType> * ret;
				_callLambda(&ret, data);
				ret->_readdNext(this);
				m_reposted = true;
			}
			catch (...)
			{
				return _reject();
			}
		}
		virtual void onKatch(PromiseRaw * from) noexcept override
		{
			std::exception_ptr * data = from->_rejectValue();
			return _rejectException(*data);
		}
	};

	template <typename InType, typename OutType, typename LAMBDA>
	class PromiseKatch<InType, Promise<OutType>*, LAMBDA> : public PromiseWithLambda<InType, Promise<OutType>*, LAMBDA>
	{
		using Super = PromiseWithLambda<InType, Promise<OutType>*, LAMBDA>;

	private:
		using PromiseRaw::_resolveCommit;
		using Super::_reject;
		using Super::_callLambdaKatch;
		using Super::_resolveValue;
		using Super::_rejectValue;
		using Super::_rejectCommit;
		bool m_reposted;

	public:
		PromiseKatch(LAMBDA&& lambda) noexcept
			:Super(move(lambda))
		{
			m_reposted = false;
		}
		PromiseKatch(const LAMBDA& lambda) noexcept
			:Super(lambda)
		{
			m_reposted = false;
		}

		virtual void onThen(PromiseRaw * from) noexcept override
		{
			try
			{
				InType * data = static_cast<Promise<InType>*>(from)->_resolveValue();
				promise_meta::with<InType, OutType>::substitution(_resolveValue(), data);
				return _resolveCommit();
			}
			catch (...)
			{
				return _reject();
			}
		}
		virtual void onKatch(PromiseRaw * from) noexcept override
		{
			if (m_reposted)
			{
				using RealOutType = unwrap_promise_t<OutType>;
				*_rejectValue() = *from->_rejectValue();
				_rejectCommit();
				return;
			}
			try
			{
				using ExType = meta::typeAt<typename meta::function<LAMBDA>::args_t, 0>;
				if (is_same_v<remove_const_t<remove_reference_t<ExType>>, exception_ptr>)
				{
					_callLambdaKatch(_resolveValue(), from->_rejectValue());
					return _resolveCommit();
				}
				else
				{
					try
					{
						std::rethrow_exception(*from->_rejectValue());
					}
					catch (ExType& ex)
					{
						Promise<OutType> * ret;
						_callLambdaKatch(_resolveValue(), &ex);
						ret->_readdNext(this);
					}
				}
			}
			catch (...)
			{
				m_reposted = true;
				return _reject();
			}
		}
	};

	template <typename ... ARGS>
	Promise<void>* PromiseRaw::all(Promise<ARGS>* ... proms) noexcept
	{
		size_t count = sizeof ... (ARGS);
		if (count == 0) return Promise<void>::resolve();
		class Prom :public Promise<void> {
		public:
			using Promise<void>::_resolve;
			size_t count = 0;
		};
		auto * prom = new Prom;
		prom->count = count;
		unpack(proms->then([prom](ARGS&) {
			prom->count--;
			if (prom->count == 0) prom->_resolve();
		}));
		return prom;
	}

	template <typename T>
	Promise<T>::~Promise() noexcept
	{
		if (m_state == Resolved)
		{
			_resolveValue()->~T();
		}
	}
	template <typename T>
	Promise<T> * Promise<T>::resolve(T &&data) noexcept
	{
		Promise<T> * prom = _new Promise<T>();
		prom->_resolve(move(data));
		return prom;
	}
	template <typename T>
	Promise<T>* Promise<T>::resolve(const T &data) noexcept
	{
		Promise<T>* prom = _new Promise<T>();
		prom->_resolve(data);
		return prom;
	}
	template <typename T>
	Promise<T> * Promise<T>::rejectException(std::exception_ptr data) noexcept
	{
		Promise<T> * prom = _new Promise<T>();
		prom->_rejectException(move(data));
		return prom;
	}
	template <typename T>
	Promise<T> * Promise<T>::reject() noexcept
	{
		return rejectException(std::current_exception());
	}
	template <typename T>
	template <typename REJTYPE>
	Promise<T> * Promise<T>::reject(REJTYPE rej) noexcept
	{
		return rejectException(make_exception_ptr(rej));
	}
	template <typename T>
	Promise<void>* Promise<T>::all(View<Promise<T>*> proms) noexcept
	{
		size_t count = proms.size();
		if (count == 0) return Promise<void>::resolve();
		class Prom :public Promise<T> {
		public:
			using Promise<T>::_resolve;
			size_t count = 0;
		};
		auto * prom = new Prom;
		prom->count = count;
		for (Promise<T> * one_of_proms : proms)
		{
			one_of_proms->then([prom](std::add_lvalue_reference_t<T>) {
				prom->count--;
				if (prom->count == 0) prom->_resolve();
			});
		}
		return prom;
	}

	template <typename T>
	template <typename LAMBDA>
	auto Promise<T>::then(LAMBDA &&lambda) noexcept->Promise<unwrap_promise_t<decltype(lambda((T&)*(T*)0))> >*
	{
		using OutType = decltype(lambda((T&)*(T*)0));
		using Prom = PromiseThen<T, OutType, decay_t<LAMBDA> >;
		using OutTypeRes = typename Prom::ResultType;
		Promise<OutTypeRes> * next = _new Prom(forward<LAMBDA>(lambda));
		_addNext(next);
		return next;
	}
	template <typename T>
	template <typename LAMBDA>
	auto Promise<T>::katch(LAMBDA &&lambda) noexcept
		->Promise<unwrap_promise_t<decltype(lambda(nullref))>>*
	{
		using OutType = decltype(lambda(zerovar));
		using Prom = PromiseKatch<T, OutType, decay_t<LAMBDA> >;
		using OutTypeRes = typename Prom::ResultType;
		Promise<OutTypeRes> * next = _new Prom(forward<LAMBDA>(lambda));
		_addNext(next);
		return next;
	}
	template <typename T>
	void Promise<T>::connect(DeferredPromise<T>* prom) noexcept
	{
		_addNext(prom);
	}
	template <typename T>
	void Promise<T>::_resolve(const T & data) noexcept
	{
		new((T*)_resolveValue()) T(data);
		PromiseRaw::_resolveCommit();
	}
	template <typename T>
	void Promise<T>::_resolve(T && data) noexcept
	{
		new((T*)_resolveValue()) T(move(data));
		PromiseRaw::_resolveCommit();
	}
	template <typename REJTYPE>
	void PromiseRaw::_reject(REJTYPE rej) noexcept
	{
		_rejectException(make_exception_ptr(rej));
	}
	template <typename T>
	T * Promise<T>::_resolveValue() noexcept
	{
		return (T*)&m_data;
	}

	template <typename LAMBDA>
	auto Promise<void>::then(LAMBDA &&lambda) noexcept->Promise<unwrap_promise_t<decltype(lambda())> >*
	{
		using OutType = decltype(lambda());
		using Prom = PromiseThen<void, OutType, decay_t<LAMBDA> >;
		using OutTypeRes = typename Prom::ResultType;
		Promise<OutTypeRes> * next = _new Prom(forward<LAMBDA>(lambda));
		_addNext(next);
		return next;
	}
	template <typename LAMBDA>
	auto Promise<void>::katch(LAMBDA &&lambda) noexcept
		->Promise<unwrap_promise_t<decltype(lambda(nullref))>>*
	{
		using OutType = decltype(lambda(zerovar));
		using Prom = PromiseKatch<void, OutType, decay_t<LAMBDA> >;
		using OutTypeRes = typename Prom::ResultType;
		Promise<OutTypeRes> * next = _new Prom(forward<LAMBDA>(lambda));
		_addNext(next);
		return next;
	}
	template <typename REJTYPE>
	PromiseVoid * Promise<void>::reject(REJTYPE rej) noexcept
	{
		return rejectException(make_exception_ptr(rej));
	}

	template <typename T>
	class DeferredPromise:public Promise<T>
	{
	public:
		using Promise<T>::_resolve;
		using Promise<T>::_reject;
		using Promise<T>::_resolveValue;
		using Promise<T>::_rejectException;
		using Promise<T>::_resolveCommit;

		virtual void onThen(PromiseRaw* from) noexcept override
		{
			try
			{
				T* data = static_cast<Promise<T>*>(from)->_resolveValue();
				promise_meta::with<T, T>::substitution(_resolveValue(), data);
				return _resolveCommit();
			}
			catch (...)
			{
				return _reject();
			}
		}
		virtual void onKatch(PromiseRaw* from) noexcept override
		{
			return _rejectException(*from->_rejectValue());
		}
	};
}