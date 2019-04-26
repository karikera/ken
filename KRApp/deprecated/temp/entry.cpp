
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <emscripten.h>

#include "library.h"
#include "main.h"

#include <iostream>

#ifndef __EMSCRIPTEN__
#define EM_ASM(x) 
#endif

int main(int argn, const char ** args)
{
	initCanvas();
	addResizeListener([](int width, int height){ 
		g_main->onResize(width, height); 
	});
	
	Main main;
	g_main = &main;

	emscripten_set_main_loop([]{ g_main->loop(); }, 0, 1);
	return 0;
}