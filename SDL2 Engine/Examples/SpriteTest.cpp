#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest()
	{
		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		testSprite = new Sprite("Assets/Textures/texture.png", glm::vec3(640, 360, 0));
		camera = new Camera(glm::vec3(0.0f, 0.0f, 4.0f));
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
		testSprite->Render();
	}
}