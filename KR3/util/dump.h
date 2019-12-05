#pragma once

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, * PEXCEPTION_POINTERS;
typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;

namespace kr
{
	class SEHException {
	public:
		PEXCEPTION_POINTERS exception;

		SEHException(PEXCEPTION_POINTERS exception) noexcept;
		Text16 getErrorText() noexcept;
		uint32_t getErrorCode() noexcept;
	};

#ifdef WIN32
	int dump(const SEHException& ex) noexcept;
#endif
	void dump_now() noexcept;
	
	class SEHCatcher
	{
	private:
		void* m_func;

	public:
		SEHCatcher() noexcept;
		~SEHCatcher() noexcept;
	};

	template <typename LAMBDA> auto dump_wrap(LAMBDA &&lambda) -> decltype(lambda())
	{
#ifdef _DEBUG
		return lambda();
#elif defined(_MSC_VER)
		SEHCatcher __catcher;
		try
		{
			return lambda();
		}
		catch(SEHException& e)
		{
			dump(e);
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
