#pragma once

//#include <mmintrin.h>  //MMX
#ifdef _M_IX86
#include <xmmintrin.h> //SSE
#elif defined(_M_AMD64)
#include <intrin.h>
#include <emmintrin.h> //SSE2
#else
#error Unsupported machine
#endif
//#include <pmmintrin.h> //SSE3
//#include <tmmintrin.h> //SSSE3
//#include <smmintrin.h> //SSE4.1
//#include <nmmintrin.h> //SSE4.2
//#include <ammintrin.h> //SSE4A
//#include <wmmintrin.h> //AES
//#include <immintrin.h> //AVX

// __builtin_shufflevector
// __builtin_convertvector
