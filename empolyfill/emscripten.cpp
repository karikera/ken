#include "emscripten.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>

constexpr int MINIMUM_MESSAGE_PROCESS_COUNT = 20;

void emscripten_set_main_loop(void(*func)(), int fps, int simulate_infinite_loop)
{
	if(fps == 0) fps = 60;
	
	if (!simulate_infinite_loop)
	{
		DebugBreak(); // TODO: Need implement
		return;
	}

	DWORD next = GetTickCount();
	DWORD term = 1000 / fps;
	MSG msg;

	for (;;)
	{
		func();
			
		DWORD now = GetTickCount();
		next += term;

		for (int i = 0; i < MINIMUM_MESSAGE_PROCESS_COUNT; i++)
		{
			if (!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) return;
		}
		long wait = next - now;
		if (wait <= 0)
		{
			next = now;
			continue;
		}
		do
		{
			if (MsgWaitForMultipleObjects(0, NULL, FALSE, wait, QS_ALLEVENTS) != WAIT_OBJECT_0)
				break;
			if (!GetMessage(&msg, nullptr, 0, 0)) return;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			wait = next - now;
		}
		while (wait > 0);
	}
}

void emscripten_set_canvas_size(int width, int height)
{
}

void emscripten_set_canvas_element_size(const char * target, int width, int height)
{
}

float emscripten_random(void)
{
	return (float)(rand() / (RAND_MAX+1));
}
