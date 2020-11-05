#pragma once

#ifdef WIN32

typedef __success(return >= 0) long HRESULT;

#else

typedef int32_t HRESULT;

#endif

namespace kr
{
	class ErrorCode
	{
	public:
		ErrorCode(HRESULT error) noexcept;
		operator HRESULT() const noexcept;
		void print() noexcept;
		template <typename C>
		TempSzText<C> getMessage() const noexcept;
		template <typename C>
		void getMessageTo(TempSzText<C>* dest) const noexcept;
		HRESULT getErrorCode() noexcept;
		static ErrorCode getLast() noexcept;

	private:
		HRESULT m_error;

	};

	class ErrorMessage
	{
	public:
		ErrorMessage(const char * error) noexcept;
		const char * getMessage() noexcept;

	private:
		const char * m_error;

	};
	template <> TSZ ErrorCode::getMessage<char>() const noexcept;
	template <> TSZ16 ErrorCode::getMessage<char16>() const noexcept;
	template <> void ErrorCode::getMessageTo<char>(TSZ*) const noexcept;
	template <> void ErrorCode::getMessageTo<char16>(TSZ16*) const noexcept;

	class FunctionError:public ErrorCode
	{
	public:
		FunctionError(const char * funcname, HRESULT error) noexcept;

		const char * getFunctionName() const noexcept;

	private:
		const char * m_funcname;
	};
	
	void errorBox(pcstr16 str) noexcept;

}

#ifdef NDEBUG
#define hrmustbe(x) {HRESULT __hr; if((__hr = (x)) < 0) exit(__hr); } while(false)
#define hrshouldbe(x) ((x) >= 0)
#define hrexcept(x)	{HRESULT __hr; if((__hr = (x)) < 0) throw ::kr::ErrorCode(__hr); } while(false)
#else
#define hrmustbe(x) {\
	HRESULT __hr = (x);\
	if((__hr) < 0) { ::kr::error(#x "\r\nHRESULT: 0x%08X\r\nMessage: %s",__hr, ErrorCode(__hr).getMessage<char>()); } \
} while(false)
#define hrshouldbe(x) ([&]{\
	HRESULT __hr = (x);\
	if((__hr) < 0){\
		::kr::dout << #x << ::kr::endl; \
		::kr::dout << __FILE__ << '(' << __LINE__ << ')' << ::kr::endl; \
		::kr::dout << "HRESULT: 0x" << ::kr::hexf((uint32_t)__hr, 8) << ::kr::endl; \
		::kr::dout << "Message: " << ErrorCode(__hr).getMessage<char>() << ::kr::endl; \
		return false; \
	}\
	return true;}())

#define hrexcept(x) {\
HRESULT __hr = (x);\
 if((__hr < 0)) {\
	::kr::dout << #x << ::kr::endl; \
	::kr::dout << __FILE__ << '(' << __LINE__ << ')' << ::kr::endl; \
	::kr::dout << "HRESULT: 0x" << ::kr::hexf((uint32_t)__hr, 8) << ::kr::endl; \
	throw ::kr::ErrorCode(__hr);\
 } } while(false)
#endif
