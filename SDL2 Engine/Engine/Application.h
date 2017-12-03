#ifndef APPLICATION
#define APPLICATION

#include <SDL.h>
#include "Window.h"
#include "Scene.h"
#include "Input.h"

class Application
{
public:
	Application();
	~Application();

	bool Run();
	void HandleEvents();
	void SetRunning(bool value);

private:
	bool running;
	SDL_Event windowEvent;
	long now, last = 0;
	float deltaTime = 0.0;
	Window* window;
	Scene* scene;
	Input* input;
};

#endif