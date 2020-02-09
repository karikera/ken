#pragma once

#define __KR3_INCLUDED

#ifdef WIN32

#ifndef WINVER
#define WINVER 0x0601 // _WIN32_WINNT_WIN7
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER
#endif
#include <SDKDDKVer.h>

#endif

#define ATTR_NO_DISCARD		[[nodiscard]]
#define ATTR_NORETURN		[[noreturn]]
#define ATTR_DEPRECATED(reason)		[[deprecated(reason)]]

#ifdef _MSC_VER

#include <sal.h>

#define ATTR_FORMAT			_Printf_format_string_
#define ATTR_NULL_TERMINATED	__nullterminated
#define ATTR_NULLABLE		_Ret_maybenull_
#define ATTR_NONULL			_Ret_notnull_
#define ATTR_INLINE			__forceinline
#define ATTR_NOALIAS		__declspec(noalias)
#define ATTR_IMPORT			__declspec(dllimport)
#define ATTR_EXPORT			__declspec(dllexport)
#define ATTR_ANY			__declspec(selectany)
#define CT_STDCALL			__stdcall
#define CT_CDECL			__cdecl
#define CT_FASTCALL			__fastcall
#define CT_VECTORCALL		__vectorcall
#define debug_force()		__debugbreak()

#ifndef NDEBUG
#define ondebug(code) code
#define debug()	debug_force()
#define _assert(x)	do {if(!(x)) {\
	fputs("Assertion failed\nOperation:" #x "\n", stderr);\
	if(::kr::checkDebugging()){\
		__debugbreak();\
	}\
	else{\
		::kr::requestDebugger();\
		__debugbreak();\
	}\
	::kr::terminate(-1);\
} } while(0,0)
#else
#define ondebug(code)
#define debug()	do {} while(0,0)
#define _assert(x)	do {__assume(x); } while(0,0)
#endif

#elif defined(__GNUG__)

#include <signal.h>

#define ATTR_NULL_TERMINATED	
#define ATTR_NULLABLE		
#define ATTR_NONULL			
#define ATTR_INLINE			__attribute__((always_inline)) inline
#define ATTR_NOALIAS		
#define ATTR_IMPORT			
#define ATTR_ANY			__attribute__((weak))
#define CT_STDCALL			
#define CT_CDECL			
#define CT_FASTCALL			
#define CT_VECTORCALL		

#ifdef __EMSCRIPTEN__
void vem__debug_break() noexcept;
#define debug_force() vem__debug_break();
#else
#define debug_force() raise(SIGTRAP)
#endif

#ifndef NDEBUG
#define ondebug(code) code
#define debug() debug_force()
#define _assert(x)	do { if(!(x)) { \
	fputs("Assertion failed\nOperation:" #x "\n", stderr);\
	debug_force(); } } while(0)
#else
#define ondebug(code)
#define debug()	do {} while(0)
#define _assert(x)	do { if (!(x)) __builtin_unreachable(); } while (0)
#endif

#else
#error Unknown compiler
#endif

#ifdef __EMSCRIPTEN__
#define NO_USE_FILESYSTEM
#define NO_USE_SOCKET
#endif


#ifdef NO_USE_SOCKET
#define NEED_SOCKET static_assert(false, "NO_USE_SOCKET");
#else
#define NEED_SOCKET
#endif
#ifdef NO_USE_FILESYSTEM
#define NEED_FILESYSTEM static_assert(false, "NO_USE_FILESYSTEM");
#else
#define NEED_FILESYSTEM
#endif

#define debugOrDie() {\
		if (::kr::requestDebugger())\
			debug();\
		::kr::terminate(-1);\
	} while(0,0)

#define notImplementedYet()	debugOrDie()
#define unreachable()		debugOrDie()

namespace kr
{
	bool checkDebugging() noexcept;
	bool requestDebugger() noexcept;
	ATTR_NORETURN void terminate(int err) noexcept;
	ATTR_NORETURN void notEnoughMemory() noexcept;
	unsigned long long rdtsc() noexcept;
}

#define EMPTY_SOURCE namespace { char __kr_pri_dummy_for_LNK4221; }
