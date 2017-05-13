#include "Engine/Engine.h"

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{	
	Engine engine;
	engine.Initialize("Lego Demo!");
	engine.GameLoop();

	SDL_DestroyWindow(Engine::getSDLWindow());
	SDL_Quit();

	return EXIT_SUCCESS;
}