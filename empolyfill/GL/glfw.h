#pragma once


int glfwInit();
int glfwOpenWindow(int width, int height, int redbits,
	int greenbits, int bluebits, int alphabits, int depthbits,
	int stencilbits, int mode);

int glfwSwapBuffers();
int glfwTerminate();

#define GL_TRUE 1
#define GL_FALSE 0


#define GLFW_WINDOW 65537