#include "Engine/Engine.h"
#include "Engine/Camera.h"
//#include "Engine/Components/Sprite.h"
#include "Engine/Components/Model.h"
#include "Engine/Components/Shader.h"

int main(int argc, char **argv)
{
	Engine engine;
	bool validInit = engine.Initialize("Lego Demo!");

	if (validInit) {

		//Sprite testSprite = Sprite("Assets/Textures/texture.png", Vector3(Engine::SCREEN_WIDTH / 2, Engine::SCREEN_HEIGHT / 2, 0));
		Shader shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		Model testModel("Assets/Models/nanosuit.obj");
		Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

		engine.SetCamera(&camera);

		long now, last = 0;
		float deltaTime = 0.0;

		while (engine.GetActive()) {

			now = SDL_GetTicks();
			deltaTime = ((float)(now - last)) / 1000;
			last = now;

			engine.Update(deltaTime);
			testModel.Update();
			engine.BeginRender();

			shader.Use();
			camera.Render(shader);
			testModel.Render(shader);
			//testSprite.Render();
			engine.EndRender();
		}
	}

	engine.~Engine();

	return EXIT_SUCCESS;
}