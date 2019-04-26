#pragma once

#include <KR3/main.h>
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
		class MySQL
		{
		public:
			/*
			id - keep reference
			password - keep reference
			db - keep reference
			*/
			MySQL(const char * host, const char * id, const char * password, const char * db, const char * charset = nullptr) noexcept;
			~MySQL() noexcept;
			MYSQL* get() noexcept;
			void ready() noexcept;
			void commit() noexcept;
			void rollback() noexcept;
			void reconnect() throws(SqlException);
			bool setCharset(Text charset) noexcept;
			void query(Text query) throws(ThrowRetry, SqlException);
			void query(MySQL & db, Text qr) throws(SqlException);
			void storeResult() throws(ThrowRetry, SqlException);
			void storeResult(MySQL & db) throws(SqlException);
			qword affectedRows() noexcept;
			qword getInsertId() noexcept;
			
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

			template <typename LAMBDA>
			inline bool _transectionSEH(LAMBDA &lambda) throws(SqlException);
			void _connect() throws(SqlException);
			void _clearResult() throws(ThrowRetry, SqlException);
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
			reconnect();
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
			ready();
			bool res = _transectionSEH(lambda);
			commit();
			return res;
		}
		catch (ThrowRetry&)
		{
			reconnect();
		}
		catch (SqlException&e)
		{
			rollback();
			throw e;
		}
	}
}
template <typename LAMBDA>
inline bool kr::sql::MySQL::_transectionSEH(LAMBDA &lambda) throws(SqlException)
{
	__try
	{
		return meta::returnBool(lambda)();
	}
	__except(1)
	{
		rollback();
		_assert(!"Transection Error");
		throw;
	}
}
