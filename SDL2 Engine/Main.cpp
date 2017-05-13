#include "Engine/Engine.h"

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{	
	Engine engine;
	engine.Initialize("Lego Demo!");
	
	SDL_Delay(10000);

	SDL_DestroyWindow(Engine::getSDLWindow());
	SDL_Quit();

	return EXIT_SUCCESS;
}