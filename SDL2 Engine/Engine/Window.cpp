#include "Window.h"

#include <glm/glm.hpp>
#include <GL/glew.h>

Window::Window(std::string title)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(0, &mode);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		std::cout << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}

	context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	GLenum result = glewInit();
	if (result != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW: " << glewGetErrorString(result) << std::endl;
	}

	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetWindowGrab(window, SDL_TRUE);
	//TODO verify the SDL_GL_SetSwapInterval setting
	SDL_GL_SetSwapInterval(1);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

Window::~Window()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

SDL_Window* Window::GetWindow()
{
	return window;
}