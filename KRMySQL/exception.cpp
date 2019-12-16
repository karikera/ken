#include "stdafx.h"

#include "mysql_include.h"
#include "exception.h"

ATTR_NORETURN void kr::sql::exception(MYSQL * mysql) throws(SqlException)
{
	warning("[MySQL][%d] %s", mysql_errno(mysql), mysql_error(mysql));
	throw SqlException();
}
ATTR_NORETURN void kr::sql::exception(MYSQL* mysql, int err) throws(SqlException)
{
	int nerr = mysql_errno(mysql);
	if (nerr == err)
	{
		warning("[MySQL][%d] %s", nerr, mysql_error(mysql));
	}
	else
	{
		warning("[MySQL][%d->%d] %s", err, nerr, mysql_error(mysql));
	}
	throw SqlException();
}
ATTR_NORETURN void kr::sql::exception(MYSQL_STMT* stmt) throws(SqlException)
{
	warning("[MySQL][%d] %s", mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
	throw SqlException();
}
ATTR_NORETURN void kr::sql::exception(MYSQL_STMT* stmt, int err) throws(SqlException)
{
	int nerr = mysql_stmt_errno(stmt);
	if (nerr == err)
	{
		warning("[MySQL][%d] %s", nerr, mysql_stmt_error(stmt));
	}
	else
	{
		warning("[MySQL][%d->%d] %s", err, nerr, mysql_stmt_error(stmt));
	}
	throw SqlException();
}
