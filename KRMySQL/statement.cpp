#include "stdafx.h"

#include "mysql_include.h"
#include <mysql/errmsg.h>
#include "statement.h"

#define KRSQL_RETRY_WRAP(cmd) \
	_assert(m_stmt->mysql == db.get());\
	for(;;){ try{ return cmd; } catch(ThrowRetry&){ db.connect(); } }

using namespace kr;

// mysql_stmt_field_count();

qword sql::PreparedStatementImpl::getInsertId() noexcept
{
	return mysql_stmt_insert_id(m_stmt);
}
qword sql::PreparedStatementImpl::affactedRows() noexcept
{
	return mysql_stmt_affected_rows(m_stmt);
}
void sql::PreparedStatementImpl::execute() throws(ThrowRetry, Exception)
{
	int err = mysql_stmt_execute(m_stmt);
	exception(m_stmt, err);
}
void sql::PreparedStatementImpl::execute(MySQL & db) throws(Exception)
{
	KRSQL_RETRY_WRAP(execute());
}

sql::PreparedStatementImpl::PreparedStatementImpl(MySQL& sql, Text query) throws(SqlException)
	:m_query(query)
{
	for (;;)
	{
		m_stmt = mysql_stmt_init(sql.get());
		if (m_stmt == nullptr) notEnoughMemory();
		int err = mysql_stmt_prepare(m_stmt, m_query.begin(), (unsigned long)m_query.size());
		try
		{
			exception(m_stmt, err);
		}
		catch (ThrowRetry&)
		{
			sql.connect();
		}
	}
}
sql::PreparedStatementImpl::~PreparedStatementImpl() noexcept
{
	mysql_stmt_close(m_stmt);
}
void sql::PreparedStatementImpl::_bindParam(MYSQL_BIND * bind) throws(Exception)
{
	int err = mysql_stmt_bind_param(m_stmt, bind);
	switch(err)
	{
	case 0: return;
	default: exception(m_stmt, err); // CR_UNSUPPORTED_PARAM_TYPE, CR_OUT_OF_MEMORY, CR_UNKNOWN_ERROR
	}
}
void sql::PreparedStatementImpl::_bindResult(MYSQL_BIND* bind) throws(Exception)
{
	int err = mysql_stmt_bind_result(m_stmt, bind);
	switch(err)
	{
	case 0: return;
	default: exception(m_stmt, err); // CR_UNSUPPORTED_PARAM_TYPE, CR_OUT_OF_MEMORY, CR_UNKNOWN_ERROR
	}
}
void sql::PreparedStatementImpl::_storeResult() throws(ThrowRetry, Exception)
{
	int err = mysql_stmt_store_result(m_stmt);
	switch (err)
	{
	case 0: return;
	case CR_SERVER_LOST:
	case CR_SERVER_GONE_ERROR: throw ThrowRetry();
	default: exception(m_stmt, err);
	}
}
void sql::PreparedStatementImpl::_storeResult(MySQL & db) throws(Exception)
{
	KRSQL_RETRY_WRAP(_storeResult());
}
void sql::PreparedStatementImpl::_freeResult() noexcept
{
	my_bool mysql_stmt_free_result_result = mysql_stmt_free_result(m_stmt);
	_assert(mysql_stmt_free_result_result == 0);
}
bool sql::PreparedStatementImpl::_fetch() throws(ThrowRetry, ThrowAllocate, Exception)
{
	switch (mysql_stmt_fetch(m_stmt))
	{
	case 0: return true;
	case 1:
	{
		int err = mysql_stmt_errno(m_stmt);
		switch (err)
		{
		case CR_SERVER_LOST:
		case CR_SERVER_GONE_ERROR: throw ThrowRetry();
		default: exception(m_stmt, err);
		}
	}
	case MYSQL_DATA_TRUNCATED: throw ThrowAllocate();
	default: // MYSQL_NO_DATA
		return false;
	}
}
bool sql::PreparedStatementImpl::_fetch(MySQL & db) throws(ThrowAllocate, Exception)
{
	KRSQL_RETRY_WRAP(_fetch());
}
void sql::PreparedStatementImpl::_fetchColumn(MYSQL_BIND * bind, kr::uint index) throws(ThrowRetry, Exception)
{
	int err = mysql_stmt_fetch_column(m_stmt, bind, index, 0);
	switch (err)
	{
	case 0: return;
	default: // CR_NO_DATA, CR_INVALID_PARAMETER_NO
		exception(m_stmt, err);
	}
}

void sql::MysqlType<Text>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
void sql::MysqlType<Text>::bindParam(MYSQL_BIND& bind, Text * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
void sql::MysqlType<AText>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
void sql::MysqlType<AText>::bindParam(MYSQL_BIND& bind, AText * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
void sql::MysqlType<AText>::initResult(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
void sql::MysqlType<AText>::bindResult(MYSQL_BIND& bind, AText * str) noexcept
{
	if (*str == nullptr) str->reserve(128);
	else str->clear();
	bind.buffer = str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->capacity());
	bind.length = (mysql_size_t*)&((size_t*)str->begin())[-1];
	bind.extension = str;
}
void sql::MysqlType<AText>::allocate(MYSQL_BIND& bind) noexcept
{
	mysql_size_t len = *bind.length;
	if(len <= bind.buffer_length) return;

	AText * str = (AText*)bind.extension;
	mysql_size_t* sizeptr = (mysql_size_t*)&((size_t*)str->begin())[-1];
	*sizeptr = bind.buffer_length;

	str->clear();
	str->reserve(len);
	bind.buffer = str->begin();
	bind.buffer_length = len;
	static_assert(sizeof(unsigned long) <= sizeof(size_t), "length size overflow");
	bind.length = (unsigned long*)&((size_t*)str->begin())[-1];
}

void sql::MysqlType<Buffer>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
void sql::MysqlType<Buffer>::bindParam(MYSQL_BIND& bind, Buffer * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
void sql::MysqlType<ABuffer>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
void sql::MysqlType<ABuffer>::bindParam(MYSQL_BIND& bind, ABuffer * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
void sql::MysqlType<ABuffer>::initResult(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
void sql::MysqlType<ABuffer>::bindResult(MYSQL_BIND& bind, ABuffer * str) noexcept
{
	if (*str == nullptr) str->reserve(128);
	else str->clear();
	bind.buffer = str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->capacity());
	static_assert(sizeof(unsigned long) <= sizeof(size_t), "length size overflow");
	bind.length = (unsigned long*)&((size_t*)str->begin())[-1];
	bind.extension = str;
}
void sql::MysqlType<ABuffer>::allocate(MYSQL_BIND& bind) noexcept
{
	mysql_size_t len = *bind.length;
	if (len <= bind.buffer_length) return;

	ABuffer * str = (ABuffer*)bind.extension;
	mysql_size_t* sizeptr = (mysql_size_t*)&((size_t*)str->begin())[-1];
	*sizeptr = bind.buffer_length;

	str->clear();
	str->reserve(len);
	bind.buffer = str->begin();
	bind.buffer_length = len;
	static_assert(sizeof(unsigned long) <= sizeof(size_t), "length size overflow");
	bind.length = (unsigned long*)&((size_t*)str->begin())[-1];
}
