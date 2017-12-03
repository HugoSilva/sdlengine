#ifndef ENGINE
#define ENGINE

#include <iostream>
#include <SDL.h>

class Window
{
public:
	Window(std::string title);
	~Window();

	SDL_Window* GetWindow();

private:
	SDL_Window* window;
	SDL_GLContext context;
	int SCREEN_WIDTH = 1280;
	int SCREEN_HEIGHT = 720;
};

#endif