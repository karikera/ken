#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/main.h>
#include <KR3/util/time.h>
#include <KR3/msg/promise.h>
#include <KR3/msg/pump.h>
#include "handle.h"
#include "devent.h"

namespace kr
{
	class EventHandle:public Handle<>
	{
		template <typename LAMBDA>
		class DispatchedEventLambda :public DispatchedEvent
		{
		private:
			LAMBDA m_lambda;

		public:
			DispatchedEventLambda(EventHandle* event, const LAMBDA& lambda) noexcept
				:DispatchedEvent(event), m_lambda(lambda)
			{
			}
			DispatchedEventLambda(EventHandle* event, LAMBDA&& lambda) noexcept
				:DispatchedEvent(event), m_lambda(move(lambda))
			{
				// regist after lambda copy
			}

			void call() noexcept override
			{
				m_lambda(this);
			}
		};

		template <typename LAMBDA>
		class DispatchedEventLambdaCurrentThread :public DispatchedEvent
		{
		private:
			LAMBDA m_lambda;
			Must<EventPump> m_pump;

		public:
			DispatchedEventLambdaCurrentThread(EventHandle* event, const LAMBDA& lambda) noexcept
				:DispatchedEvent(event), m_lambda(lambda), m_pump(EventPump::getInstance())
			{
			}
			DispatchedEventLambdaCurrentThread(EventHandle* event, LAMBDA&& lambda) noexcept
				:DispatchedEvent(event), m_lambda(lambda), m_pump(EventPump::getInstance())
			{
			}

			void call() noexcept override
			{
				AddRef();
				m_pump->post([this]() {
					if (!canceled())
					{
						m_lambda(this);
					}
					Release();
					});
			}
		};
	public:
		EventHandle() = delete;
		EventHandle(const EventHandle&) = delete;
		EventHandle& operator =(const EventHandle&) = delete;
		static EventHandle * create(bool status, bool manual) noexcept;
		static void operator delete(ptr event) noexcept;
		void wait() noexcept;
		// true: this is setted
		// false: timeout
		bool wait(duration timer) noexcept;
		// true: this is setted
		// false: ev is setted
		bool waitWith(EventHandle * ev) noexcept;
		// 1: this is setted
		// 0: ev is setted
		// -1: timeout
		int waitWith(EventHandle * ev, duration ms) noexcept;
		void set() noexcept;
		void reset() noexcept;

		template <typename LAMBDA>
		DispatchedEvent* callback(LAMBDA&& lambda) noexcept
		{
			using DEvent = DispatchedEventLambdaCurrentThread<decay_t<LAMBDA> >;
			DEvent* ev = _new DEvent(this, forward<LAMBDA>(lambda));
			ev->_regist();
			return ev;
		}
		template <typename LAMBDA>
		DispatchedEvent* callbackThreaded(LAMBDA&& lambda) noexcept
		{
			using DEvent = DispatchedEventLambda<decay_t<LAMBDA> >;
			DEvent* ev = _new DEvent(this, forward<LAMBDA>(lambda));
			ev->_regist();
			return ev;
		}
		Promise<void>* promise() noexcept;
		Promise<void>* promiseAndRemove() noexcept;
	};

	class SemaphoreHandle:public EventHandle
	{
	public:
		SemaphoreHandle() = delete;
		~SemaphoreHandle() = delete;

		static SemaphoreHandle * create(long initsem, long maxsem) noexcept;
		bool release(long count = 1) noexcept;

	private:
		using EventHandle::set;
		using EventHandle::reset;
	};

	class Event
	{
	public:
		static constexpr dword TIMEOUT = 258L;

		Event(EventHandle * handle) noexcept;
		explicit Event(bool bStatus = false, bool manual = false) noexcept;
		Event(Event && move) noexcept;
		~Event() noexcept;
		Event(const Event&) = delete;
		Event& operator = (const Event&) = delete;
		EventHandle * operator ->() noexcept;
		EventHandle *& operator &() noexcept;

		Event & operator = (Event && move) noexcept;
		Event & operator = (nullptr_t) noexcept;

		bool operator == (nullptr_t) noexcept;
		bool operator == (const Event & _other) noexcept;
		bool operator != (nullptr_t) noexcept;
		bool operator != (const Event & _other) noexcept;

	protected:
		EventHandle * m_event;
	};

	template <typename T>
	class DataEvent
	{
	private:
		Event m_send, m_end;
		T m_value;

	public:
		inline DataEvent() noexcept
			:m_end(true)
		{
		}
		inline T wait() noexcept
		{
			m_send->wait();
			T value = move(m_value);
			m_end->set();
			return value;
		}
		inline void reset() noexcept
		{
			m_end->set();
			m_send->reset();
		}
		inline bool wait(duration time, T* dest) noexcept
		{
			if(!m_send->wait(time)) return false;
			*dest = move(m_value);
			m_end->set();
			return true;
		}
		inline void send(T value) noexcept
		{
			m_end->wait();
			m_value = move(value);
			m_send->set();
		}

		// when this is setted: true
		// when ev is setted: false
		inline bool waitWith(EventHandle * ev, T* dest) noexcept
		{
			if(!m_send->waitWith(ev)) return false;
			*dest = move(m_value);
			m_end->set();
			return true;
		}
		// when this is setted: true
		// when ev is setted: false
		inline bool waitWith(EventHandle * ev, duration ms, T* dest) noexcept
		{
			if (!m_send->waitWith(ev, ms)) return false;
			*dest = move(m_value);
			m_end->set();
			return true;
		}
		inline EventHandle * getEventHandle() noexcept
		{
			return &m_send;
		}

	};

	namespace _pri_
	{
		class EventListImpl:public Bufferable<EventListImpl, BufferInfo<EventHandle*> >
		{
		public:
			EventListImpl() noexcept;
			dword wait() noexcept;
			dword wait(dword time) noexcept;
			EventHandle * pickOut(size_t idx) noexcept;
			void pop() noexcept;
			EventHandle * const * $begin() const noexcept;
			EventHandle * const * $end() const noexcept;
			size_t $size() const noexcept;
			void clear() noexcept;

		protected:
			EventHandle ** _ptr() noexcept;
			EventHandle * const * _ptr() const noexcept;
			size_t m_size;
		};
	}

	template <size_t SIZE> class EventList:public _pri_::EventListImpl
	{
	public:	
		EventList() = default;
		EventList(initializer_list<EventHandle*> ev) noexcept
		{
			m_size = 0;
			push(ev);
		}
		EventList(View<EventHandle*> ev) noexcept
		{
			m_size = 0;
			push(ev);
		}
		void push(EventHandle * ev) noexcept
		{
			_assert(m_size < SIZE);
			m_events[m_size ++] = ev;
		}

		void push(View<EventHandle*> ev) noexcept
		{
			_assert(m_size + ev.size() <= SIZE);
			memcpy(m_events + m_size, ev.begin(), sizeof(EventHandle*) * ev.size());
			m_size += ev.size();
		}

	private:
		EventHandle * m_events[SIZE];
	};

	class EventWaiter:public Event
	{
	public:
		EventWaiter(bool bStatus = false) noexcept;
		~EventWaiter() noexcept;
	};
}
