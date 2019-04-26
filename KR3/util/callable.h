#pragma once

namespace kr
{
	template <typename func_t> class CallableT;
	template <typename func_t, typename LAMBDA> class LambdaCallable;
	template <typename func_t> class CallablePtrT;
	template <typename func_t> struct LambdaTable;
	template <size_t sz, typename func_t> class Lambda;
	template <typename func_t>
	class CallableListT;

	using Callable = CallableT<void()>;
	using CallablePtr = CallablePtrT<void()>;
	using CallableList = CallableListT<void()>;

	template <typename RET, typename ... ARGS>
	class CallableT<RET(ARGS...)>
	{
	public:
		CallableT() noexcept
		{
		}
		virtual ~CallableT() noexcept
		{
		}
		virtual RET operator ()(ARGS ... args) = 0;
		RET call(ARGS ... args)
		{
			return (*this)(args ...);
		}

		template <typename LAMBDA>
		static CallableT * wrap(LAMBDA lambda);
	};

	template <typename RET, typename ... ARGS, typename LAMBDA> 
	class LambdaCallable<RET(ARGS...), LAMBDA>:public CallableT<RET(ARGS...)>
	{
	private:
		LAMBDA m_lambda;

	public:
		inline LambdaCallable(LAMBDA lambda)
			:m_lambda(move(lambda))
		{
		}
		~LambdaCallable() noexcept override
		{
		}
		RET operator ()(ARGS ... args) override
		{
			return m_lambda(args ...);
		}
	};

	template <typename RET, typename ... ARGS>
	class CallablePtrT<RET(ARGS...)>
	{
	private:
		CallableT<RET(ARGS...)> * m_callable;

	public:
		CallablePtrT() noexcept = default;
		CallablePtrT(CallableT<RET(ARGS...)> * _callable) noexcept
		{
			m_callable = _callable;
		}
		template <typename LAMBDA>
		CallablePtrT(LAMBDA _lambda)
			:CallablePtrT(CallableT<RET(ARGS...)>::wrap(move(_lambda)))
		{
		}
		operator CallableT<RET(ARGS...)>* () const noexcept
		{
			return m_callable;
		}
		CallableT<RET(ARGS...)> & operator *() const noexcept
		{
			return *m_callable;
		}
		CallableT<RET(ARGS...)> * operator ->() const noexcept
		{
			return m_callable;
		}
	};

	template <typename RET, typename ... ARGS>
	class CallableListT<RET(ARGS ...)>
	{
	private:
		Array<CallableT<RET(ARGS ...)>*> m_list;

	public:
		CallableT<RET(ARGS ...)> * add(CallablePtrT<RET(ARGS ...)> cb) noexcept
		{
			CallableT<RET(ARGS ...)> * pcb = cb;
			reline_new(pcb);
			m_list.push(pcb);
			return pcb;
		}
		bool detach(CallableT<RET(ARGS ...)> * cb) noexcept
		{
			return m_list.removeMatch(cb);
		}
		bool remove(CallableT<RET(ARGS ...)> * cb) noexcept
		{
			if (m_list.removeMatch(cb))
			{
				delete cb;
				return true;
			}
			return false;
		}
		void clear() noexcept
		{
			for (auto * func : m_list)
			{
				delete func;
			}
			m_list.clear();
		}
		void fire(ARGS ... args) noexcept
		{
			for (auto * func : m_list)
			{
				func->call(args ...);
			}
		}
	};

	template <typename RET, typename ... ARGS>
	template <typename LAMBDA>
	CallableT<RET(ARGS...)> * CallableT<RET(ARGS...)>::wrap(LAMBDA lambda)
	{
		return _new LambdaCallable<RET(ARGS...), LAMBDA>(move(lambda));
	}

	template <typename RET, typename ... ARGS>
	struct LambdaTable<RET(ARGS ...)>
	{
		RET(*call)(const ARGS & ..., void*);
		void(*destruct)(void*);
		void(*copy)(void*, const void*);
		void(*move)(void*, const void*);

		template <typename LAMBDA>
		static const LambdaTable<RET(ARGS ...)> * get() noexcept;
	};

	template <size_t sz, typename RET, typename ... ARGS>
	class Lambda<sz, RET(ARGS ...)>
	{
	public:
		Lambda() noexcept;
		Lambda(const Lambda & _copy) noexcept;
		Lambda(Lambda && _move) noexcept;
		template <typename LAMBDA>
		Lambda(LAMBDA lambda) noexcept;
		~Lambda() noexcept;
		void clear() noexcept;
		bool empty() noexcept;
		Lambda& operator =(const Lambda & _copy) noexcept;
		Lambda& operator =(Lambda && _move) noexcept;
		RET operator ()(ARGS ... args);

	private:
		const LambdaTable<RET(ARGS ...)>* _getEmptyTable() noexcept;

		const LambdaTable<RET(ARGS ...)> * m_table;
		byte m_lambda[sz];
	};

	template <typename RET, typename ... ARGS>
	template <typename LAMBDA>
	const LambdaTable<RET(ARGS ...)> * LambdaTable<RET(ARGS ...)>::get() noexcept
	{
		static const LambdaTable<RET(ARGS ...)> vtable = {
			([](const ARGS &... args, void * lambda) noexcept->RET { return (*(LAMBDA*)lambda)(args ...); }),
			([](void * lambda) noexcept { ((LAMBDA*)lambda)->~LAMBDA(); }),
			([](void * dest, const void * src) noexcept {
				new((LAMBDA*)dest) LAMBDA(*(LAMBDA*)src);
			}),
			([](void * dest, const void * src) noexcept {
				new((LAMBDA*)dest) LAMBDA(kr::move(*(LAMBDA*)src));
			})
		};
		return &vtable;
	}

	template <size_t sz, typename RET, typename ... ARGS>
	Lambda<sz, RET(ARGS ...)>::Lambda() noexcept
	{
		m_table = _getEmptyTable();
	}
	template <size_t sz, typename RET, typename ... ARGS>
	Lambda<sz, RET(ARGS ...)>::Lambda(const Lambda & _copy) noexcept
	{
		m_table = _copy.m_table;
		m_table->copy(m_lambda, _copy.m_lambda);
	}
	template <size_t sz, typename RET, typename ... ARGS>
	Lambda<sz, RET(ARGS ...)>::Lambda(Lambda && _move) noexcept
	{
		m_table = _move.m_table;
		m_table->move(m_lambda, _move.m_lambda);
	}
	template <size_t sz, typename RET, typename ... ARGS>
	template <typename LAMBDA>
	Lambda<sz, RET(ARGS ...)>::Lambda(LAMBDA lambda) noexcept
	{
		static_assert(sizeof(LAMBDA) <= sizeof(m_lambda), "Lambda size unmatch");
		m_table = LambdaTable<RET(ARGS ...)>::template get<LAMBDA>();
		new((LAMBDA*)m_lambda) LAMBDA(lambda);
	}
	template <size_t sz, typename RET, typename ... ARGS>
	Lambda<sz, RET(ARGS ...)>::~Lambda() noexcept
	{
		m_table->destruct(m_lambda);
	}
	template <size_t sz, typename RET, typename ... ARGS>
	void Lambda<sz, RET(ARGS ...)>::clear() noexcept
	{
		m_table->destruct(m_lambda);
		m_table = _getEmptyTable();
	}
	template <size_t sz, typename RET, typename ... ARGS>
	bool Lambda<sz, RET(ARGS ...)>::empty() noexcept
	{
		return m_table == _getEmptyTable();
	}
	template <size_t sz, typename RET, typename ... ARGS>
	Lambda<sz, RET(ARGS ...)>& Lambda<sz, RET(ARGS ...)>::operator =(const Lambda & _copy) noexcept
	{
		m_table->destruct(m_lambda);
		m_table = _copy.m_table;
		m_table->copy(m_lambda, _copy.m_lambda);
		return *this;
	}
	template <size_t sz, typename RET, typename ... ARGS>
	Lambda<sz, RET(ARGS ...)>& Lambda<sz, RET(ARGS ...)>::operator =(Lambda && _move) noexcept
	{
		m_table->destruct(m_lambda);
		m_table = _move.m_table;
		m_table->move(m_lambda, _move.m_lambda);
		return *this;
	}
	template <size_t sz, typename RET, typename ... ARGS>
	RET Lambda<sz, RET(ARGS ...)>::operator ()(ARGS ... args)
	{
		return m_table->call(args ..., m_lambda);
	}
	template <size_t sz, typename RET, typename ... ARGS>
	const LambdaTable<RET(ARGS ...)>* Lambda<sz, RET(ARGS ...)>::_getEmptyTable() noexcept
	{
		static const LambdaTable<RET(ARGS ...)> emptyTable = {
			([](const ARGS & ..., void *) noexcept->RET { return RET(); }),
			([](void *) noexcept {}),
			([](void * dest, const void * src) noexcept {}),
			([](void * dest, const void * src) noexcept {})
		};
		return &emptyTable;
	}

	using LambdaP = Lambda<sizeof(void*), void()>;

}