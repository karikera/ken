#pragma once

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;
extern "C" void *        __cdecl _exception_info(void);

namespace kr
{
	namespace _pri_
	{
#ifdef WIN32
		int dump(LPEXCEPTION_POINTERS ex) noexcept;
#endif
	}
	void dump_now() noexcept;

	template <typename LAMBDA> auto dump_wrap(LAMBDA lambda) -> decltype(lambda())
	{
#ifdef _DEBUG
		return lambda();
#elif defined(_MSC_VER)
		__try
		{
			return lambda();
		}
		__except(_pri_::dump((LPEXCEPTION_POINTERS)_exception_info()))
		{
			return zerovar.value<decltype(lambda())>();
		}
#else
		try
		{
			return lambda();
		}
		catch (...)
		{
			dump_now();
			return zerovar.value<decltype(lambda())>();
		}
#endif
	}
}
