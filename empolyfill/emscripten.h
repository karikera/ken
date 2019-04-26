#pragma once

#define DEPRECATED __declspec(deprecated)

void emscripten_set_main_loop(void(*func)(), int fps, int simulate_infinite_loop);
void emscripten_set_canvas_size(int width, int height);
void emscripten_set_canvas_element_size(const char * target, int width, int height);
float emscripten_random(void);


//emscripten_set_keydown_callback

typedef int EM_BOOL;
typedef char EM_UTF8;

typedef int emscripten_align1_int;

#ifdef __EMSCRIPTEN__
#define EM_ASM(x)	
#define EM_ASM_(x, ...)	(__VA_ARGS__)
#define EM_ASM_INT(x, ...)	((__VA_ARGS__),(int)0)
#define EM_ASM_INT_V(x)	((int)0)
#define EM_ASM_DOUBLE(x, ...)	((__VA_ARGS__),(double)0)
#define EM_ASM_DOUBLE_V(x)	((double)0)
#else
#define EM_ASM(x)		static_assert(false,"Cannot call EM_ASM!")
#define EM_ASM_(x, ...)	static_assert(false,"Cannot call EM_ASM!")
#define EM_ASM_INT(x, ...)	static_assert(false,"Cannot call EM_ASM!")
#define EM_ASM_INT_V(x)	static_assert(false,"Cannot call EM_ASM!")
#define EM_ASM_DOUBLE(x, ...)	static_assert(false,"Cannot call EM_ASM!")
#define EM_ASM_DOUBLE_V(x)	static_assert(false,"Cannot call EM_ASM!")
#endif
