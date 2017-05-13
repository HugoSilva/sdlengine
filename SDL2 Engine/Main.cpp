#include "Engine/Engine.h"
#include "Engine/Components/Sprite.h"


#include <iostream>
using namespace std;

int main(int argc, char **argv)
{	
	Engine engine;
	bool validInit = engine.Initialize("Lego Demo!");

	if (validInit) {

		Sprite testSprite = Sprite("Assets/Textures/texture.png", Vector3(Engine::SCREEN_WIDTH / 2, Engine::SCREEN_HEIGHT / 2, 0));

		while (engine.GetActive()) {
			engine.Update();
			engine.BeginRender();
			testSprite.Render();
			engine.EndRender();
		}
	}

	engine.~Engine();

	return EXIT_SUCCESS;
}