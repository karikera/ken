#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/mt/thread.h>
#include <KR3/win/windows.h>
#include <KR3/win/eventhandle.h>
#include <KR3/data/map.h>
#include <atomic>

#include "handle.h"

namespace kr
{
	class ProcessDebugger:public Threadable<ProcessDebugger>
	{
	public:
		ProcessDebugger() noexcept;
		~ProcessDebugger() noexcept;
		int thread() noexcept;

		void watch(ProcessId process, CallablePtrT<void(DEBUG_EVENT*)> listener) noexcept;
		void unwatch(ProcessId process) noexcept;
		void continueDebug(ProcessId process, ThreadId thread, dword code) noexcept;

	private:
		Event m_insertWait;

		enum FuncType
		{
			FuncNone,
			FuncDebugActiveProcess,
			FuncDebugActiveProcessStop,
			FuncContinueDebugEvent,
			FuncExit
		};

		struct CallbackInfo
		{
			CallableT<void(DEBUG_EVENT*)> * callback;
		};

		// call params
		std::atomic<FuncType> m_call;
		ProcessId m_processId;
		dword m_threadId;
		dword m_continue;

		Map<dword, CallbackInfo> m_callbackList;
	};
}
