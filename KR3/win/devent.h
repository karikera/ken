#pragma once

#include <KR3/main.h>

namespace kr
{
	class DispatchedEventCommon:protected VCounter
	{
	public:
		DispatchedEventCommon(EventHandle* event) noexcept;
		virtual ~DispatchedEventCommon() noexcept;

		EventHandle* handle() const noexcept;
		virtual void call() noexcept = 0;

	protected:
		Must<EventPump> m_pump;
		EventHandle* const m_event;
	};
	
	class DispatchedEventWinImpl:public DispatchedEventCommon
	{
	public:
		DispatchedEventWinImpl(EventHandle* event) noexcept;
		~DispatchedEventWinImpl() noexcept;
		void cancel() noexcept;
		bool canceled() noexcept;

		void _regist() noexcept;

	private:
		void* m_waiter;
	};

	class DispatchedEventKrImpl:public DispatchedEventCommon
	{
		friend Referencable<DispatchedEventKrImpl>;
	public:
		class Manager;

		DispatchedEventKrImpl(EventHandle * event) noexcept;
		virtual ~DispatchedEventKrImpl() noexcept;
		void cancel() noexcept;
		bool canceled() noexcept;

		void _regist() noexcept;

	private:
		Manager* m_dispatcher;
	};

	using DispatchedEvent = DispatchedEventWinImpl;
}
