#include "Application.h"

Application::Application()
{
	window = new Window("Lego Demo");
	scene = new Scene();
	//create renderer
	//control inputs
	running = true;
}

Application::~Application()
{
}

bool Application::Run()
{
	now = SDL_GetTicks();
	deltaTime = ((float)(now - last)) / 1000;
	last = now;
	scene->Update();

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene->Render();

	SDL_GL_SwapWindow(window->GetWindow());

	return running;
}