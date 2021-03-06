#include "stdafx.h"

#include "mysql_include.h"
#include "db.h"
#include "statement.h"
#include <mysql/errmsg.h>

#pragma comment(lib, "libmariadb.lib")

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

kr::sql::Result::Result() noexcept
	:m_res(nullptr)
{
}
kr::sql::Result::Result(nullptr_t) noexcept
	:m_res(nullptr)
{
}
kr::sql::Result::Result(MYSQL_RES* res) noexcept
	:m_res(res)
{
}
bool kr::sql::Result::isEmpty() const noexcept
{
	return m_res == nullptr;
}
void kr::sql::Result::close() const noexcept
{
	mysql_free_result(m_res);
}
MYSQL_ROW kr::sql::Result::fetch() const noexcept
{
	return mysql_fetch_row(m_res);
}
kr::Text kr::sql::Result::getName(uint idx) const noexcept
{
	MYSQL_FIELD* field = mysql_fetch_field_direct(m_res, idx);
	return Text(field->name, field->name_length);
}

MYSQL * kr::sql::MySQL::get() noexcept
{
	return m_conn;
}
kr::sql::MySQL::MySQL(_In_opt_ const char * host, _In_opt_ const char * id, _In_opt_ const char * password, _In_opt_ const char * db, _In_opt_ const char * charset, int port) noexcept
	:m_host(host),m_id(id),m_password(password),m_db(db), m_charset(charset), m_port(port)
{
	m_conn = mysql_init(nullptr);
	if (m_conn == nullptr) notEnoughMemory();
}
kr::sql::MySQL::~MySQL() noexcept
{
	mysql_close(m_conn);
}

void kr::sql::MySQL::autocommit(bool enabled) noexcept
{
	mysql_autocommit(m_conn, enabled);
}
void kr::sql::MySQL::commit() noexcept
{
	mysql_commit(m_conn);
}
void kr::sql::MySQL::rollback() noexcept
{
	mysql_rollback(m_conn);
}
void kr::sql::MySQL::connect() throws(Exception)
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
	int err = mysql_real_query(m_conn, query.begin(), intact<dword>(query.size()));
	exception(m_conn, err);
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
			connect();
		}
	}
}
bool kr::sql::MySQL::nextResult() throws(ThrowRetry, Exception)
{
	int err = mysql_next_result(m_conn);
	if (err == -1) return false;
	exception(m_conn, err);
	return true;
}
void kr::sql::MySQL::clearResult() throws(ThrowRetry, Exception)
{
	for (;;)
	{
		MYSQL_RES* res = mysql_use_result(m_conn);
		mysql_free_result(res);
		int err = mysql_next_result(m_conn);
		if (err == -1) return;
		exception(m_conn, err);
	}
}
kr::sql::Result kr::sql::MySQL::useResult() throws(ThrowRetry, Exception)
{
	return mysql_use_result(m_conn);
}
kr::sql::Result kr::sql::MySQL::useResult(MySQL& db) throws(SqlException)
{
	_assert(&db == this);
	for (;;)
	{
		try
		{
			return useResult();
		}
		catch (ThrowRetry&)
		{
			connect();
		}
	}
}
kr::sql::Result kr::sql::MySQL::storeResult() throws(ThrowRetry, Exception)
{
	return mysql_store_result(m_conn);
}
kr::sql::Result kr::sql::MySQL::storeResult(MySQL & db) throws(SqlException)
{
	_assert(&db == this);
	for (;;)
	{
		try
		{
			return storeResult();
		}
		catch (ThrowRetry&)
		{
			connect();
		}
	}
}
kr::uint kr::sql::MySQL::fieldCount() noexcept
{
	return mysql_field_count(m_conn);
}
kr::qword kr::sql::MySQL::affectedRows() noexcept
{
	return mysql_affected_rows(m_conn);
}
kr::qword kr::sql::MySQL::getInsertId() noexcept
{
	return mysql_insert_id(m_conn);
}
int kr::sql::MySQL::getErrorNumber() noexcept
{
	return mysql_errno(m_conn);
}
const char* kr::sql::MySQL::getErrorMessage() noexcept
{
	return mysql_error(m_conn);
}

void kr::sql::MySQL::_connect() throws(SqlException)
{
	if (mysql_real_connect(m_conn, m_host, m_id, m_password, m_db, m_port, nullptr, CLIENT_MULTI_STATEMENTS) == nullptr)
	{
		ondebug(
			int err = mysql_errno(m_conn);
			warning("[MySQL][%d] %s", err, mysql_error(m_conn));
		);
		throw SqlException();
	}

	if(m_charset == nullptr) return;
	bool setCharsetResult = setCharset((Text)m_charset);
	_assert(setCharsetResult);
}