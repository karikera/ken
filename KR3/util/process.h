#pragma once

#include "../main.h"
#include "../io/bufferedstream.h"

namespace kr
{
	class Process : public InStream<Process, char>
	{
	public:
		enum Shell_t { Shell };
		Process() noexcept;
		Process(Shell_t, Text16 command) noexcept;
		Process(pcstr16 fileName, pstr16 parameter) noexcept;
		~Process() noexcept;

		void close() noexcept;
		void shell(Text16 command, pcstr16 curdir = nullptr) throws(Error);
		void exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir = nullptr) throws(Error);
		void exec(pstr16 commandLine) throws(Error);
		size_t readImpl(char * dest, size_t sz) throws(EofException);
		void wait() noexcept;
		bool wait(int millis) noexcept;
#ifdef WIN32
		// true: process is done
		// false: canceler is setted
		bool waitWith(EventHandle * canceler) noexcept;
		EventHandle * getEventHandle() noexcept;
#endif

		int getExitCode() noexcept;
		void * getCloseEventHandle() noexcept;

#ifdef WIN32
		static void executeOpen(pcstr16 path) noexcept;
		static AText call(pstr16 path) throws(ThrowAbort, Error);
		static AText call(pstr16 path, EventHandle * canceler) throws(ThrowAbort, Error);
#else
		static void executeOpen(pcstr path) noexcept;
#endif

		static int execute(pstr16 pszstr) noexcept;

		// return zero when it succeeded
		static int detachedExecute(pstr16 pszstr) noexcept;

	private:
#ifdef WIN32
		void* m_process;
		void* m_stdout_read;
#endif

	};

	class ProcessId
	{
	public:
		ProcessId() = default;
		ProcessId(dword id) noexcept;
		static ProcessId findByName(Text16 name) noexcept;
		static TmpArray<ProcessId> findsByName(Text16 name) noexcept;

		dword value() noexcept;

	private:
		dword m_id;
	};
}
