#pragma once

#include <KR3/main.h>

namespace kr
{
	struct ModuleInfo
	{
		pcstr16 name;
	};

	struct StackInfo
	{
		void * address;
		pcstr16 filename;
		pcstr function;
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
		virtual void onDbgHelpErr(pcstr function, dword gle, qword addr) noexcept;
		virtual void onOutput(Text16 szText) noexcept = 0;

#ifdef WIN32
		void* m_hProcess;
		dword m_dwProcessId;

		bool m_modulesLoaded;
#else

#endif
	};


}
