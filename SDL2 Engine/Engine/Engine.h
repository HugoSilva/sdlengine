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
	static SDL_Window* getSDLWindow();
	static float GetDT();

	Engine();
	~Engine();

	bool Initialize(char* windowTitle);
	void GameLoop();
	void Update();
	void BeginRender();
	void EndRender();

private:
	static SDL_Window* window;
	static SDL_Renderer* renderer;
	static float dt;

	float lastTime;
};

#endif
