#pragma once

#include "../main.h"
#include "../meta/number.h"

typedef struct st_mysql_stmt MYSQL_STMT;
typedef struct st_mysql MYSQL;

namespace kr
{
	enum class HttpStatus :int;

	using std::current_exception;
	using std::exception_ptr;
	using std::make_exception_ptr;

	struct Exception {};
	struct UnknownException : Exception {};
	struct ThrowAbort : Exception {};
	struct ThrowRetry : Exception {};
	struct ThrowAllocate : Exception {};
	struct DuplicateException : Exception {};
	struct NotFoundException : Exception {};
	struct NotEnoughSpaceException : Exception {};
	struct EofException : Exception {};
	struct OutOfRangeException : Exception {};
	struct InvalidSourceException : Exception {};
	struct TooBigException : Exception {};
	struct UnsupportedException : Exception {};
	struct DisconnectException : Exception {};
	struct HttpException : Exception {
		HttpStatus status;
		inline HttpException(HttpStatus value) noexcept :status(value) {}
	};
	struct QuitException : Exception {
		int exitCode;
		inline QuitException(int value) noexcept :exitCode(value) {}
	};
	struct Error : Exception {};
	struct SqlException : Exception {};
	struct MciError : Error {
		uint value;
		inline MciError(uint value) noexcept :value(value) {}
	};
	struct ZlibError :Error {
		int value;
		inline ZlibError(int value) noexcept :value(value) {}
	};

	ATTR_NORETURN void quit(int exitCode) throws(QuitException);
	ATTR_NORETURN void error(const char * strMessage, ...) noexcept;
	void warning(const char * strMessage, ...) noexcept;

	template <typename T, typename T2> bool intactTest(T2 v) noexcept
	{
		if (std::is_signed_v<T2> && std::is_unsigned_v<T>)
		{
			if (v < 0) return false;
		}
		else if (std::is_signed_v<T> && std::is_unsigned_v<T2>)
		{
			if ((T)v < 0) return false;
		}
		return (T2)(T)v == v;
	}
	template <typename T, typename T2> T intact(T2 v) noexcept
	{
		_assert(intactTest<T>(v));
		return (T)v;
	}
	template <typename T>
	class IntactAuto
	{
	private:
		T m_value;

	public:
		IntactAuto(T value) noexcept
			:m_value(value)
		{
		}

		template <typename T2>
		operator T2() noexcept
		{
			return intact<T2>(m_value);
		}
	};

	template <typename T> IntactAuto<T> intactAuto(T v) noexcept
	{
		return v;
	}
}

#define shouldbe(cmp)	((cmp) ? true : (::kr::warning(#cmp), false))
#define mustbe(cmp)		{ if(!(cmp)) ::kr::error(#cmp); }