#pragma once

#include <KR3/main.h>

struct alignas(16) _CONTEXT;
typedef _CONTEXT CONTEXT;

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
		bool showCallstack(CONTEXT * ctx) noexcept;

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

	class StackWriter:public HasStreamTo<StackWriter, char16>, private StackWalker
	{
	private:
		io::VOStream<char16> m_out;
		CONTEXT* const m_ctx;
		
	public:
		StackWriter(CONTEXT * ctx = nullptr) noexcept;

		template <typename Derived, typename Info>
		void $streamTo(OutStream<Derived, char16, Info>* target) noexcept
		{
			m_out = target;
			if (m_ctx) showCallstack(m_ctx);
			else showCallstack();
		}

	private:
		void onOutput(Text16 szText) noexcept override;
	};
	


}
