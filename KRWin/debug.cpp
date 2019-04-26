#include "stdafx.h"

using namespace kr;

#ifdef WIN32
#include "debug.h"

#include <tchar.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <process.h>
#include <unordered_map>

#pragma comment(lib,"psapi.lib")

ProcessDebugger::ProcessDebugger() noexcept
	:m_call(FuncNone), m_insertWait(true)
{
	start();
}
ProcessDebugger::~ProcessDebugger() noexcept
{
	m_insertWait->wait();
	m_call = FuncExit;
	join();
}
int ProcessDebugger::thread() noexcept
{
	for (;;)
	{
		if (m_call != FuncNone)
		{
			switch (m_call)
			{
			case FuncDebugActiveProcess:
				DebugActiveProcess(m_processId.value());
				break;
			case FuncDebugActiveProcessStop:
			{
				DebugActiveProcessStop(m_processId.value());
				auto v = m_callbackList.find(m_processId.value());
				if (v != m_callbackList.end()) m_callbackList.erase(v);
				break;
			}
			case FuncContinueDebugEvent:
				ContinueDebugEvent(m_processId.value(), m_threadId, m_continue);
				break;
			case FuncExit:
				return 0;
			}
			m_call = FuncNone;
			m_insertWait->set();
		}

		DEBUG_EVENT de;
		if (!WaitForDebugEvent(&de, 200)) continue;

		m_callbackList[de.dwProcessId].callback->call(&de);
	}
}

void ProcessDebugger::watch(ProcessId process, CallablePtrT<void(DEBUG_EVENT*)> listener) noexcept
{
	m_callbackList.insert({
		process.value(),
		CallbackInfo{
			listener
		}
	});

	m_insertWait->wait();
	m_processId = process;
	m_call = FuncDebugActiveProcess;
}
void ProcessDebugger::unwatch(ProcessId process) noexcept
{
	if (isCurrentThread())
	{
		DebugActiveProcessStop(process.value());
	}
	else
	{
		m_insertWait->wait();
		m_processId = process;
		m_call = FuncDebugActiveProcessStop;
	}
}
void ProcessDebugger::continueDebug(ProcessId process, ThreadId thread, dword code) noexcept
{
	if(isCurrentThread())
	{
		ContinueDebugEvent(process.value(), thread.value(), code);
	}
	else
	{
		m_insertWait->wait();
		m_processId = process;
		m_threadId = thread.value();
		m_continue = code;
		m_call = FuncContinueDebugEvent;
	}
}

#endif