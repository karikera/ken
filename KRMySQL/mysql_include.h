#pragma once

#ifdef __MYSQL_REFERENCE_H
#error include before mysql_ref.h 
#endif

#include <math.h>
#include <time.h>

#pragma warning(push)
#pragma warning(disable:4996)
#undef _WIN32_WINNT
#include <mysql/mysql.h>
#pragma warning(pop)
