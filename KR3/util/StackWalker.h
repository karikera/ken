#pragma once

#include <KR3/main.h>

struct _CONTEXT;
typedef _CONTEXT CONTEXT;

namespace kr
{
	struct ModuleInfo
	{
		pcstr16 name;
	};

	struct StackInfo
	{
		void* base;
		void* address;
		pcstr16 moduleName;
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
		bool showCallstack(CONTEXT * ctx, void* threadHandle) noexcept;

	protected:
		virtual void onLoadModule(ModuleInfo* info) noexcept;
		virtual void onStack(StackInfo * entry) noexcept = 0;
		virtual void onDbgHelpErr(pcstr function, dword gle, qword addr) noexcept = 0;

#ifdef WIN32
		void* m_hProcess;
		dword m_dwProcessId;

		bool m_modulesLoaded;
#else

#endif
	};


	class StringStackWalker:public StackWalker
	{
	protected:
		virtual void onStack(StackInfo* entry) noexcept override;
		virtual void onDbgHelpErr(pcstr function, dword gle, qword addr) noexcept override;
		virtual void onOutput(Text16 szText) noexcept = 0;

#ifdef WIN32
		void* m_hProcess;
		dword m_dwProcessId;

		bool m_modulesLoaded;
#else

#endif
	};

	class StackWriter:public HasStreamTo<StackWriter, char16>, private StringStackWalker
	{
	private:
		io::VOStream<char16> m_out;
		CONTEXT* const m_ctx;
		void* const m_threadHandle;
		
	public:
		StackWriter(CONTEXT * ctx = nullptr, void* threadHandle = nullptr) noexcept;

		template <typename Derived, typename Info>
		void $streamTo(OutStream<Derived, char16, Info>* target) noexcept
		{
			m_out = target;
			if (m_ctx) showCallstack(m_ctx, m_threadHandle);
			else showCallstack();
		}

	private:
		void onOutput(Text16 szText) noexcept override;
	};

	Array<StackInfo> getStackInfos(CONTEXT* ctx = nullptr, void* threadHandle = nullptr) noexcept;

}
