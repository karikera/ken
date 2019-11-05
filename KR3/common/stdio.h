#pragma once

namespace kr
{
	enum class ConsoleType
	{
		Standard,
		Error,
		Debug
	};
	template <ConsoleType type, typename C>
	class ConsoleOutputStream;
	template <typename C>
	using StandardOutputT = ConsoleOutputStream<ConsoleType::Standard, C>;
	template <typename C>
	using StandardErrorOutputT = ConsoleOutputStream<ConsoleType::Error, C>;
	template <typename C>
	using DebugOutputT = ConsoleOutputStream<ConsoleType::Debug, C>;
	using StandardOutput = StandardOutputT<char>;
	using StandardOutput16 = StandardOutputT<char16>;
	using StandardErrorOutput = StandardErrorOutputT<char>;
	using StandardErrorOutput16 = StandardErrorOutputT<char16>;
	using DebugOutput = DebugOutputT<char>;
	using DebugOutput16 = DebugOutputT<char16>;


	template <typename Derived, typename Component, typename Info = StreamInfo<false, Empty> >
	class FlushOutStream : public OutStream<Derived, Component, Info>
	{
	public:
		template <typename _Derived, typename _Info>
		void passThrough(InStream<_Derived, Component, _Info> * is)
		{
			try
			{
				Pipe<FlushOutStream, InStream<_Derived, Component, _Info>> pipe(this, is);
				for (;;)
				{
					pipe.streaming();
					static_cast<Derived*>(this)->flush();
				}
			}
			catch (EofException&)
			{
			}
		}
	};

	template <ConsoleType type, typename C>
	class ConsoleOutputStream: public FlushOutStream<ConsoleOutputStream<type, C>, C>
	{
	public:
		void flush() noexcept;
		void $write(const C *chr, size_t sz) noexcept;

		static ConsoleOutputStream out;
	};

	template <typename C>
	class ConsoleOutputStream<ConsoleType::Debug, C> :
		public FlushOutStream<ConsoleOutputStream<ConsoleType::Debug, C>, C>
	{
	public:
		void $write(const C *chr, size_t sz) noexcept;
		void putSourceLine(const C * src, int line) noexcept;
		void flush() noexcept;

		static ConsoleOutputStream out;
	};
#ifdef WIN32
	template <>
	class ConsoleOutputStream<ConsoleType::Debug, char16> :
		public FlushOutStream<ConsoleOutputStream<ConsoleType::Debug, char16>, char16>
	{
	public:
		ConsoleOutputStream() noexcept;
		~ConsoleOutputStream() noexcept;
		void $write(const char16 * chr, size_t sz) noexcept;
		void putSourceLine(pcstr16 src, int line) noexcept;
		void flush() noexcept;

		static ConsoleOutputStream<ConsoleType::Debug, char16> out;

	private:
#ifndef NDEBUG
		int _thread() noexcept;
#endif
	};
	
#else
	template <>
	void DebugOutput::putSourceLine(pcstr src, int line) noexcept;
	template <>
	void DebugOutput::flush() noexcept;
#endif

	extern StandardOutput &cout;
	extern StandardOutput16 &ucout;
	extern StandardErrorOutput &cerr;
	extern StandardErrorOutput16 &ucerr;
	extern DebugOutput &dout;
	extern DebugOutput16 &udout;

	void dumpMemory(void * addr, size_t size) noexcept;

	template <ConsoleType type, typename C>
	ConsoleOutputStream<type,C> ConsoleOutputStream<type, C>::out;

	template <typename C>
	ConsoleOutputStream<ConsoleType::Debug, C> ConsoleOutputStream<ConsoleType::Debug, C>::out;

	template <>
	void DebugOutput::$write(const char *chr, size_t sz) noexcept;
	template <>
	void StandardOutput::flush() noexcept;
	template <>
	void StandardOutput::$write(const char * chr, size_t sz) noexcept;
	template <>
	void StandardOutput16::flush() noexcept;
	template <>
	void StandardOutput16::$write(const char16 * chr, size_t sz) noexcept;
	template <>
	void StandardErrorOutput::flush() noexcept;
	template <>
	void StandardErrorOutput::$write(const char * chr, size_t sz) noexcept;
	template <>
	void StandardErrorOutput16::flush() noexcept;
	template <>
	void StandardErrorOutput16::$write(const char16 * chr, size_t sz) noexcept;

	template <ConsoleType type, typename C>
	inline void ConsoleOutputStream<type, C>::flush() noexcept
	{
		ConsoleOutputStream<type, char>::flush();
	}
	template <ConsoleType type, typename C>
	inline void ConsoleOutputStream<type, C>::$write(const C *chr, size_t sz) noexcept
	{
		ConsoleOutputStream<type, char>::out << (ToAcp<C>)View<C>(chr, sz);
	}
	template <typename C>
	inline void ConsoleOutputStream<ConsoleType::Debug, C>::$write(const C *chr, size_t sz) noexcept
	{
#ifdef WIN32
		if (sizeof(C) == sizeof(char16))
			udout << Text16((const char16*)chr, sz);
		else
			dout << toAnsi(View<C>(chr, sz));
#else
		dout << toAnsi(View<C>(chr, sz));
#endif
	}
	template <typename C>
	inline void ConsoleOutputStream<ConsoleType::Debug, C>::putSourceLine(const C * src, int line) noexcept
	{
		*this << src << (C)'(' << line << (C)')' << (C)'\r' << (C)'\n';
	}
	template <typename C>
	inline void ConsoleOutputStream<ConsoleType::Debug, C>::flush() noexcept
	{
#ifdef WIN32
		udout.flush();
#else
		dout.flush();
#endif
	}

}