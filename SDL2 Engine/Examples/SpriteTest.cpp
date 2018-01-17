#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		audio::SoundManager::add(new audio::Sound("eff", "effect.wav"));
		//m_Music = Mix_LoadMUS("background.ogg");

		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");

		testTexture[0] = new Texture("tex1.jpg");
		testTexture[1] = new Texture("tex2.png");
		testTexture[2] = new Texture("tex3.png");
		testTexture[3] = new Texture("tex4.png");

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
				Sprite* iter = new Sprite(glm::vec3(i * 40, j * 40, 0), glm::vec2(40, 40), testTexture[rand() % 4]);
				m_Renderables.push_back(iter);
			}
		}

		GLint texIDs[] =
		{
			0, 1, 2, 3
		};

		shader->Enable();
		shader->setUniform1iv("textures", texIDs, 4);

		audio::SoundManager::get("eff")->play();
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