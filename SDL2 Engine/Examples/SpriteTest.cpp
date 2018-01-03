#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		//"Assets/Textures/texture.png"
		//string directory = imagePath.substr(0, imagePath.find_last_of('/'));
		//Texture* texture = new Texture(imagePath.c_str(), directory, "");

		//renderer = new graphics::SDLRenderer(win);
		renderer = new graphics::OpenGLRenderer(win);

		for (int i = 0; i < 210; i++ )
		{
			for (int j = 0; j < 119; j++)
			{
				float randNum = rand() % 1000 / 1000.0f;
				Sprite* iter = new Sprite(glm::vec3(i*6, j*6, 0), glm::vec2(5, 5), glm::vec4(randNum/1.1f, 0.9f, randNum, 1));
				m_Renderables.push_back(iter);
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
		camera->Render(*shader);
		shader->Enable();

		renderer->begin();

		for (const graphics::Renderable2D* renderable : m_Renderables)
			renderable->submit(renderer);

		renderer->end();
		renderer->flush();
	}
}