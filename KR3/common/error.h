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