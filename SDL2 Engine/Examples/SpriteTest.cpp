#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		//"Assets/Textures/texture.png"
		//string directory = imagePath.substr(0, imagePath.find_last_of('/'));
		//Texture* texture = new Texture(imagePath.c_str(), directory, "");

		renderer = new Engine::Renderer2D(win);

		for (int i = 0; i < 210; i++ )
		{
			for (int j = 0; j < 119; j++)
			{
				int randNum = rand() % (255 - 100 + 1) + 100;
				Sprite* iter = new Sprite(glm::vec3(i*6, j*6, 0), glm::vec2(5, 5), glm::vec4(1, 1, randNum, 255));
				renderer->submit(iter);
			}
		}

		camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	SpriteTest::~SpriteTest()
	{
	}

	void SpriteTest::Update(float deltaTime)
	{
		camera->Update(deltaTime);
	}

	void SpriteTest::Render()
	{
		shader->Use();
		camera->Render(*shader);

		renderer->begin();
		renderer->flush();
		renderer->end();
	}
}