#include "Engine.h"
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

int Engine::SCREEN_WIDTH = 1280;
int Engine::SCREEN_HEIGHT = 720;
SDL_Window* Engine::window = NULL;
SDL_Renderer* Engine::renderer = NULL;
float Engine::dt = 0;

Engine::Engine()
{

}

Engine::~Engine()
{

}

bool Engine::Initialize(char* windowTitle)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

	if (window == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return false;
	}

	return true;
}

void Engine::GameLoop()
{
	SDL_Event windowEvent;

	while (true) {
		if (SDL_PollEvent(&windowEvent)) {
			switch (windowEvent.type) {
			case SDL_QUIT:
				return;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				Keyboard::KeyCallback(windowEvent.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				Mouse::MouseButtonCallback(windowEvent.button);
			case SDL_MOUSEMOTION:
				Mouse::MousePosCallback(windowEvent.motion);
				break;
			}
		}
	}
}

void Engine::Update()
{
}

void Engine::BeginRender()
{
}

void Engine::EndRender()
{
	SDL_RenderClear(this->renderer);
}

SDL_Window* Engine::getSDLWindow()
{
	return window;
}

float Engine::GetDT()
{
	return dt;
}