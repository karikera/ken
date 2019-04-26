#include "common.h"
#include "fstream.h"

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

AText16 g_filename;

ErrMessage::ErrMessage(const char * msg, ...) noexcept
{
	va_list vl;
	va_start(vl, msg);
	vsprintf_s(m_message, msg, vl);
}
void ErrMessage::print() noexcept
{
	ucerr << g_filename << u'(' << g_is.getLine() << u"): error " << acpToUtf16((Text)m_message) << endl;
}

void must(char chr, char must)
{
	if(chr == must) return;
	throw ErrMessage("Need \'%c\' not \'%c\'", must, chr);
}
