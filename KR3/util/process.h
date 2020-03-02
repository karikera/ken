#pragma once

#include "../main.h"
#include "../io/bufferedstream.h"

namespace kr
{
	class Process;

	class StdStream:public InStream<StdStream, char>
	{
		friend Process;
	public:
		StdStream() noexcept;
		StdStream(StdStream&& _move) noexcept;
		~StdStream() noexcept;
		size_t $read(char* dest, size_t sz) throws(EofException);
		void close() noexcept;

	private:
#ifdef WIN32
		void* m_stream_read;
#endif
	};
	class Process
	{
	public:
		enum Shell_t { Shell };
		Process() noexcept;
		Process(Process&& _move) noexcept;
		Process(Shell_t, Text16 command) noexcept;
		Process(pstr16 command) noexcept;
		Process(pcstr16 fileName, pstr16 parameter, pcstr16 curdir = nullptr, StdStream* out = nullptr, StdStream* err = nullptr) noexcept;
		~Process() noexcept;

		void close() noexcept;
#ifdef WIN32
		void cmd(pstr16 parameter, pcstr16 curdir = nullptr, StdStream* out = nullptr, StdStream* err = nullptr) throws(Error);
#endif
		void shell(Text16 command, pcstr16 curdir = nullptr, StdStream* out = nullptr, StdStream* err = nullptr) throws(Error);
		void exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir = nullptr, StdStream* out = nullptr, StdStream* err = nullptr) throws(Error);
		void exec(pstr16 commandLine) throws(Error);
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


	class ProcessStream :public InStream<ProcessStream, char>
	{
	public:
		Process process;
		StdStream stream;

		ProcessStream() noexcept;
		ProcessStream(ProcessStream&& _move) noexcept;
		~ProcessStream() noexcept;

		size_t $read(char* dest, size_t sz) throws(EofException);
		void close() noexcept;
#ifdef WIN32
		void cmd(pstr16 parameter, pcstr16 curdir = nullptr) throws(Error);
#endif
		void shell(Text16 command, pcstr16 curdir = nullptr) throws(Error);
		void exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir = nullptr) throws(Error);
		void exec(pstr16 commandLine) throws(Error);
	};

	StreamBuffer<char, ProcessStream> shell(Text16 command, pcstr16 curdir = nullptr) throws(Error);
	void shellPiped(Text16 command, pcstr16 curdir = nullptr) throws(Error);
	StreamBuffer<char, ProcessStream> exec(pcstr16 file, pstr16 parameter, pcstr16 curdir = nullptr) noexcept;
	StreamBuffer<char, ProcessStream> exec(Text16 command) noexcept;
	StreamBuffer<char, ProcessStream> exec(pstr16 command) noexcept;
	StreamBuffer<char, ProcessStream> exec(pstr16 path, EventHandle* canceler) throws(ThrowAbort, Error);
	void execPiped(pstr16 path) throws(ThrowAbort, Error);
	void execOpen(pcstr16 path) noexcept;
	int execNoOutput(pstr16 pszstr) noexcept;
	int execDetached(pstr16 pszstr) noexcept;
}
