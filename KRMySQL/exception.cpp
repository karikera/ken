#include "stdafx.h"

#include "mysql_include.h"
#include "exception.h"

ATTR_NORETURN void kr::sql::exception(MYSQL * con) throws(SqlException)
{
	warning("[MySQL][%d] %s", mysql_errno(con), mysql_error(con));
	throw SqlException();
}
ATTR_NORETURN void kr::sql::exception(MYSQL_STMT* stmt) throws(SqlException)
{
	warning("[MySQL][%d] %s", mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
	throw SqlException();
}
