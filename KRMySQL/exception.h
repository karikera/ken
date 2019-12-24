#pragma once

#include "mysql_ref.h"

namespace kr
{
	namespace sql
	{
		void exception(MYSQL* mysql, int err) throws(ThrowRetry, SqlException);
		void exception(MYSQL_STMT* stmt, int err) throws(ThrowRetry, SqlException);
	}
}