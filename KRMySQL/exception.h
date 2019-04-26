#pragma once

#include "mysql_ref.h"

namespace kr
{
	namespace sql
	{
		ATTR_NORETURN void exception(MYSQL * mysql) throws(SqlException);
		ATTR_NORETURN void exception(MYSQL_STMT * stmt) throws(SqlException);
	}
}