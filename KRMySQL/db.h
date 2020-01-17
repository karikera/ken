#pragma once

#include <KR3/main.h>
#include <KR3/meta/chreturn.h>
#include "mysql_ref.h"
#include "exception.h"

namespace kr
{

	namespace sql
	{
		class MySQLServer
		{
		public:
			MySQLServer() noexcept;
			~MySQLServer() noexcept;
		};
		class Result
		{
		public:
			Result() noexcept;
			Result(nullptr_t) noexcept;
			Result(MYSQL_RES* res) noexcept;
			bool isEmpty() const noexcept;
			void close() const noexcept;
			MYSQL_ROW fetch() const noexcept;
			Text getName(uint idx) const noexcept;

		private:
			MYSQL_RES* m_res;
		};
		class MySQL
		{
		public:
			/*
			id - keep reference
			password - keep reference
			db - keep reference
			*/
			MySQL(_In_opt_ const char * host, _In_opt_ const char * id, _In_opt_ const char * password, _In_opt_ const char * db = nullptr, _In_opt_ const char * charset = nullptr, int port = 0) noexcept;
			~MySQL() noexcept;
			MySQL(const MySQL&) = delete;
			MYSQL* get() noexcept;
			void autocommit(bool enabled) noexcept;
			void commit() noexcept;
			void rollback() noexcept;
			void connect() throws(SqlException);
			bool setCharset(Text charset) noexcept;
			void query(Text query) throws(ThrowRetry, SqlException);
			void query(MySQL & db, Text qr) throws(SqlException);
			bool nextResult() throws(ThrowRetry, Exception);
			void clearResult() throws(ThrowRetry, SqlException);
			Result useResult() throws(ThrowRetry, SqlException);
			Result useResult(MySQL& db) throws(SqlException);
			Result storeResult() throws(ThrowRetry, SqlException);
			Result storeResult(MySQL & db) throws(SqlException);
			uint fieldCount() noexcept;
			qword affectedRows() noexcept;
			qword getInsertId() noexcept;
			int getErrorNumber() noexcept;
			const char * getErrorMessage() noexcept;
			
			template <typename LAMBDA>
			inline bool connection(LAMBDA &lambda) throws(SqlException);
			template <typename LAMBDA>
			inline bool transection(LAMBDA &lambda) throws(SqlException);

		private:
			MYSQL * m_conn;
			const char * const m_host;
			const char * const m_id;
			const char * const m_password;
			const char * const m_db;
			const char * const m_charset;
			const int m_port;

			void _connect() throws(SqlException);
		};

	}
}
		
template <typename LAMBDA>
inline bool kr::sql::MySQL::connection(LAMBDA &lambda) throws(SqlException)
{
	for (;;)
	{
		try
		{
			return meta::returnBool(lambda)();
		}
		catch (ThrowRetry&)
		{
			connect();
		}
		catch (SqlException&e)
		{
			throw e;
		}
	}
}
template <typename LAMBDA>
inline bool kr::sql::MySQL::transection(LAMBDA &lambda) throws(SqlException)
{
	for (;;)
	{
		try
		{
			autocommit(false);
			bool res = meta::returnBool(lambda)();
			commit();
			autocommit(true);
			return res;
		}
		catch (ThrowRetry&)
		{
			connect();
		}
		catch (SqlException&e)
		{
			rollback();
			autocommit(true);
			throw e;
		}
		catch (...)
		{
			rollback();
			autocommit(true);
			debug();
			throw;
		}
	}
}
