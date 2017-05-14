#include "Engine/Engine.h"
#include "Engine/Components/Sprite.h"
//#include "Engine/Components/Model.h"
//#include "Engine/Components/Shader.h"

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{	
	Engine engine;
	bool validInit = engine.Initialize("Lego Demo!");

	if (validInit) {

		Sprite testSprite = Sprite("Assets/Textures/texture.png", Vector3(Engine::SCREEN_WIDTH / 2, Engine::SCREEN_HEIGHT / 2, 0));
		//if (GLEW_OK != glewInit())
		//{
		//	std::cout << "Failed to initialize GLEW" << std::endl;
		//	return EXIT_FAILURE;
		//}

		//Shader shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		//Model model("Assets/Models/stormtrooper.fbx");

		while (engine.GetActive()) {
			engine.Update();
			engine.BeginRender();
			testSprite.Render();
			//model.Draw(shader);
			engine.EndRender();
		}
	}

	engine.~Engine();

	return EXIT_SUCCESS;
}