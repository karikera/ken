#include "stdafx.h"
#include "dynamic_dbghelp.h"

DbgHelp* DbgHelp::getInstance() noexcept
{
	static DbgHelp dbghelp;
	return &dbghelp;
}

DbgHelp::DbgHelp() noexcept
{
}
