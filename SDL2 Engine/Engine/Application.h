#ifndef APPLICATION
#define APPLICATION

#include <SDL.h>
#include "Window.h"
#include "Scene.h"

class Application
{
public:
	Application();
	~Application();

	bool Run();

private:
	bool running;
	long now, last = 0;
	float deltaTime = 0.0;
	Window* window;
	Scene* scene;
};

#endif