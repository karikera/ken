#pragma once


#ifdef CBSEASYV8_EXPORTS
#define KR_EASYV8_DLLEXPORT __declspec(dllexport)
#else
#define KR_EASYV8_DLLEXPORT __declspec(dllimport)
#endif

namespace kr
{

	KR_EASYV8_DLLEXPORT void * ezv8_malloc(size_t sz);
	KR_EASYV8_DLLEXPORT void ezv8_free(void * ptr);

}