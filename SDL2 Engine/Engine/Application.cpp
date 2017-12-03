#include "Application.h"

Application::Application()
{
	window = new Window("Lego Demo");
	scene = new Scene();
	//create renderer
	input = new Input();
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
	this->HandleEvents();
	scene->Update(deltaTime);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene->Render();

	SDL_GL_SwapWindow(window->GetWindow());

	return running;
}

void Application::HandleEvents()
{
	if (SDL_PollEvent(&windowEvent)) {
		switch (windowEvent.type) {
		case SDL_QUIT:
			this->SetRunning(false);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			input->HandleKeyboard(windowEvent.key);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			input->HandleMouseButton(windowEvent.button);
		case SDL_MOUSEMOTION:
			input->HandleMouseMotion(windowEvent.motion);
			break;
		}
	}
}

void Application::SetRunning(bool value)
{
	running = value;
}