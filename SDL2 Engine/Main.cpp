#include "Engine/Engine.h"
//#include "Engine/Components/Sprite.h"
#include "Engine/Camera.h"
#include "Engine/Components/Model.h"
#include "Engine/Components/Shader.h"

#include <iostream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

int main(int argc, char **argv)
{
	Engine engine;
	bool validInit = engine.Initialize("Lego Demo!");

	if (validInit) {

		//Sprite testSprite = Sprite("Assets/Textures/texture.png", Vector3(Engine::SCREEN_WIDTH / 2, Engine::SCREEN_HEIGHT / 2, 0));
		Shader shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		Model testModel("Assets/Models/nanosuit.obj");
		float i = 0.0f;

		engine.SetCamera(&camera);
		glm::mat4 projection = glm::perspective(camera.GetZoom(), 1280.0f/720.0f, 0.1f, 100.0f);


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

			glm::mat4 view = camera.GetViewMatrix();
			glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

			testModel.Draw(shader);

			//testSprite.Render();
			engine.EndRender();
		}
	}

	engine.~Engine();

	return EXIT_SUCCESS;
}