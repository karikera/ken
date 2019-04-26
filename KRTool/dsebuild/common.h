#pragma once

#include <KR3/main.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

using namespace kr;
using std::bad_alloc;
using std::unordered_map;
using std::list;
using std::string;
using std::wstring;
using std::hash;
using std::ifstream;
using std::ofstream;
using kr::Text;


class ErrMessage
{
public:
	ErrMessage(ATTR_FORMAT const char * msg, ...) noexcept;
	void print() noexcept;

private:
	char m_message[1024];
};

extern AText16 g_filename;

void must(char chr, char must);
