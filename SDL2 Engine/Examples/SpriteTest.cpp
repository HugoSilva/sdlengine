#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		//"Assets/Textures/texture.png"
		//string directory = imagePath.substr(0, imagePath.find_last_of('/'));
		//Texture* texture = new Texture(imagePath.c_str(), directory, "");

		testSprite = new Sprite(glm::vec3(10, 10, 0), glm::vec2(200, 200), glm::vec4(1, 1, 1, 1));
		camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));

		renderer = new Engine::Renderer2D(win);
		renderer->submit(testSprite);
	}

	SpriteTest::~SpriteTest()
	{
	}

	void SpriteTest::Update(float deltaTime)
	{
		camera->Update(deltaTime);
		testSprite->Update();
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