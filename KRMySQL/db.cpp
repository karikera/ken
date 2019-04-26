#include "stdafx.h"

#include "mysql_include.h"
#include "db.h"
#include "statement.h"
#include <errmsg.h>

#pragma comment(lib, "libmysql.lib")

#define SQLEXCEPTION() sql::SQLException(__FILE__ ":%d",__LINE__)
#define SQLSTMTEXCEPTION(stmt) sql::SQLStatementException(stmt,__FILE__ ":%d",__LINE__)

using namespace std;

kr::sql::MySQLServer::MySQLServer() noexcept
{
	int mysql_server_init_result = mysql_server_init(0, nullptr, nullptr);
	_assert(mysql_server_init_result == 0);
}
kr::sql::MySQLServer::~MySQLServer() noexcept
{
	mysql_server_end();
}

MYSQL * kr::sql::MySQL::get() noexcept
{
	return m_conn;
}
kr::sql::MySQL::MySQL(const char * host, const char * id, const char * password, const char * db, const char * charset) noexcept
	:m_host(host),m_id(id),m_password(password),m_db(db), m_charset(charset)
{
	try
	{
		m_conn = mysql_init(nullptr);
		if(m_conn == nullptr) exception(m_conn);
		_connect();
	}
	catch(SqlException&)
	{
		error("SQL Connection Initializing failed.");
	}
}
kr::sql::MySQL::~MySQL() noexcept
{
	mysql_close(m_conn);
}

void kr::sql::MySQL::ready() noexcept
{
	mysql_autocommit(m_conn, 0);
}
void kr::sql::MySQL::commit() noexcept
{
	mysql_commit(m_conn);
}
void kr::sql::MySQL::rollback() noexcept
{
	mysql_rollback(m_conn);
}
void kr::sql::MySQL::reconnect() throws(Exception)
{
	_connect();
}
bool kr::sql::MySQL::setCharset(Text charset) noexcept
{
	BText<64> query;
	query << "set names " << charset;
	return mysql_real_query(m_conn, query.begin(), intact<dword>(query.size())) == 0;
}
void kr::sql::MySQL::query(Text query) throws(ThrowRetry, Exception)
{
	switch (mysql_real_query(m_conn, query.begin(), intact<dword>(query.size())))
	{
	case 0: _clearResult(); return;
	case CR_SERVER_GONE_ERROR:
	case CR_SERVER_LOST:
		throw ThrowRetry();
	default: // CR_COMMANDS_OUT_OF_SYNC, CR_UNKNOWN_ERROR
		exception(m_conn);
	}
}
void kr::sql::MySQL::query(MySQL & db, Text qr) throws(Exception)
{
	_assert(&db == this);
	for (;;)
	{
		try
		{
			return query(qr);
		}
		catch (ThrowRetry&)
		{
			reconnect();
		}
	}
}
void kr::sql::MySQL::storeResult() throws(ThrowRetry, Exception)
{
	//do {
	//	MYSQL_RES * result = mysql_store_result(m_conn);
	//	if (result)
	//	{
	//		process_result_set(m_conn, result);
	//		mysql_free_result(result);
	//	}
	//	else
	//	{
	//		if (mysql_field_count(m_conn) == 0)
	//		{
	//			printf("%lld rows affected\n",
	//				mysql_affected_rows(m_conn));
	//		}
	//		else
	//		{
	//			printf("Could not retrieve result set\n");
	//			break;
	//		}
	//	}
	//	if ((status = mysql_next_result(m_conn)) > 0)
	//		printf("Could not execute statement\n");
	//}
	//while (status == 0);
	notImplementedYet();
}
void kr::sql::MySQL::storeResult(MySQL & db) throws(SqlException)
{
	_assert(&db == this);
	notImplementedYet();
}
kr::qword kr::sql::MySQL::affectedRows() noexcept
{
	return mysql_affected_rows(m_conn);
}
kr::qword kr::sql::MySQL::getInsertId() noexcept
{
	return mysql_insert_id(m_conn);
}

void kr::sql::MySQL::_connect() throws(Exception)
{
	if (mysql_real_connect(m_conn, m_host, m_id, m_password, m_db, m_host ? 3066 : 0, nullptr, CLIENT_MULTI_STATEMENTS) == nullptr)
	{
		exception(m_conn);
	}

	if(m_charset == nullptr) return;
	bool setCharsetResult = setCharset((Text)m_charset);
	_assert(setCharsetResult);
}
void kr::sql::MySQL::_clearResult() throws(ThrowRetry, Exception)
{
	for (;;)
	{
		MYSQL_RES * res = mysql_use_result(m_conn);
		mysql_free_result(res);
		switch (mysql_next_result(m_conn))
		{
		case 0: break;
		case -1: return;
		case CR_SERVER_GONE_ERROR:
		case CR_SERVER_LOST:
			throw ThrowRetry();
		default: // CR_COMMANDS_OUT_OF_SYNC, CR_UNKNOWN_ERROR
			exception(m_conn);
		}
	}
}