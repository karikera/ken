#include "fstream.h"

#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

#include <KRUtil/stdext.h>

using namespace std::krext;

LangParser g_is(LangParser::DEFAULT_WORD_SPLITTER);
LangWriter g_os;
