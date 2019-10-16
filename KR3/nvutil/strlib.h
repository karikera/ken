
#pragma once

#include "ncstrlib.h"

inline size_t CRtoCRLF(char * out,const char * str,size_t limit)
{
	char * out_end = out + limit-1;
	char * out_start = out;

	LPCSTR end = strchr(str,'\0');
	LPCSTR prev = str,next;

	while(next = (char*)memchr(prev,'\n',end-prev))
	{
		size_t skip = next-prev;
		if(out+skip+2 >= out_end)
		{
			*out = '\0';
			return out - out_start;
		}

		memcpy(out,prev,skip);
		out += skip;
		*(out++) = '\r';
		*(out++) = '\n';
		
		prev = next+1;
	}
	memcpy(out,prev,end-prev+1);
	
	return out-prev +end-out_start;
}
