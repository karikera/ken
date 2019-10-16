#pragma once

#include <vector>
#include <string>

#ifdef _UNICODE
typedef std::wstring _tstring;
#define _FTS "%ls"
#else
typedef std::string _tstring;
#define _FTS "%hs"
#endif

template <typename T> bool vector_eject(std::vector<T> &v,const T &value)
{
	auto end = v.end();
	for(auto i = v.begin();i != end;i++)
	{
		if((*i) != value) continue;
		v.erase(i);
		return true;
	}
	return false;
}

typedef std::basic_ostream<char,std::char_traits<char>> char_ostream;

inline char_ostream & operator <<(char_ostream &file,LPCWSTR str)
{
	char temp[1024];
	int len = WideCharToMultiByte(CP_ACP,0,str,wcslen(str),temp,1023,nullptr,nullptr);
	temp[len] = '\0';
	file << temp;
	return file;
}
