#ifndef ENGINE
#define ENGINE

#include "../Libs/SDL2/include/SDL.h"
#pragma comment(lib, "opengl32.lib")

#include <iostream>
using namespace std;

class Engine
{
public:
	static int SCREEN_WIDTH;
	static int SCREEN_HEIGHT;
	static SDL_Window* GetWindow();
	static SDL_Renderer* GetRenderer();
	static SDL_Surface* GetSurface();

	Engine();
	~Engine();

	bool Initialize(char* windowTitle);
	void Update();
	void BeginRender();
	void EndRender();
	bool GetActive();

private:
	static SDL_Window* window;
	static SDL_Renderer* renderer;
	static SDL_Surface* surface;
	bool active;
};

#endif
