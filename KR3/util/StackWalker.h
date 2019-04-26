#pragma once

#include <KR3/main.h>

namespace kr
{
	struct ModuleInfo
	{
		const char * name;
	};

	struct StackInfo
	{
		void * address;
		const char * filename;
		const char * function;
		unsigned int line;
	};

	class StackWalker
	{
	public:
		StackWalker() noexcept;
		~StackWalker() noexcept;

		bool loadModules() noexcept;
		bool showCallstack() noexcept;

	protected:
		virtual void onLoadModule(ModuleInfo * info) noexcept;
		virtual void onStack(StackInfo * entry) noexcept;
		virtual void onDbgHelpErr(pcstr szFuncName, dword gle, qword addr) noexcept;
		virtual void onOutput(pcstr szText) noexcept = 0;

#ifdef WIN32
		void* m_hProcess;
		dword m_dwProcessId;

		bool m_modulesLoaded;
#else

#endif
	};


}
