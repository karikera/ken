#include "stdafx.h"

#include "mysql_include.h"
#include "exception.h"

#include <mysql/errmsg.h>

void kr::sql::exception(MYSQL* mysql, int err) throws(ThrowRetry, SqlException)
{
	if (err == -1) err = mysql_errno(mysql);
	switch (err)
	{
	case 0: return;
	case CR_SERVER_LOST:
	case CR_SERVER_GONE_ERROR: throw ThrowRetry();
	default:
		warning("[MySQL][%d] %s", err, mysql_error(mysql));
		throw SqlException();
	}
}
void kr::sql::exception(MYSQL_STMT* stmt, int err) throws(ThrowRetry, SqlException)
{
	if (err == -1) err = mysql_stmt_errno(stmt);
	switch (err)
	{
	case 0: return;
	case CR_SERVER_LOST:
	case CR_SERVER_GONE_ERROR: throw ThrowRetry();
	default:
		warning("[MySQL][%d] %s", err, mysql_stmt_error(stmt));
		throw SqlException();
	}
}
