#include "Engine.h"
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

int Engine::SCREEN_WIDTH = 1280;
int Engine::SCREEN_HEIGHT = 720;
SDL_Window* Engine::window = NULL;
SDL_Renderer* Engine::renderer = NULL;
SDL_Surface* Engine::surface = NULL;

Engine::Engine()
{

}

Engine::~Engine()
{
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool Engine::Initialize(char* windowTitle)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); 
	surface = SDL_GetWindowSurface(window);

	if (window == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return false;
	}

	return true;
}

void Engine::Update()
{
	SDL_Event windowEvent;

	if (SDL_PollEvent(&windowEvent)) {
		switch (windowEvent.type) {
		case SDL_QUIT:
			active = false;
			break;
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

void Engine::BeginRender()
{
	//SDL_RenderClear(renderer);
}

void Engine::EndRender()
{
	//SDL_RenderPresent(renderer);
}

SDL_Window* Engine::GetWindow()
{
	return window;
}

SDL_Renderer* Engine::GetRenderer()
{
	return renderer;
}

SDL_Surface* Engine::GetSurface()
{
	return surface;
}

bool Engine::GetActive()
{
	return this->active;
}