#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");

		testTexture = new Texture("C:/Projects/sdlengine/test.png");

		//renderer = new graphics::SDLRenderer(win);
		renderer = new graphics::OpenGLRenderer(win);

		for (int i = 0; i < 32; i++ )
		{
			for (int j = 0; j < 18; j++)
			{
				int r = rand() % 200;
				int g = rand() % 256;
				int b = rand() % 210;

				unsigned int color = 0xff << 24 | b << 16 | g << 8 | r;

				//Sprite* iter = new Sprite(glm::vec3(i*40, j*40, 0), glm::vec2(40, 40), color);
				Sprite* iter = new Sprite(glm::vec3(i * 40, j * 40, 0), glm::vec2(40, 40), testTexture);
				m_Renderables.push_back(iter);
			}
		}

		GLint texIDs[] =
		{
			0
		};

		shader->Enable();
		shader->setUniform1iv("textures", texIDs, 1);

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