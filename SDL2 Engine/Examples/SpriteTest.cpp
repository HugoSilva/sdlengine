#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		audio::SoundManager::add(new audio::Sound("eff", "effect.wav"));
		audio::SoundManager::add(new audio::Music("bgm", "background.ogg"));

		shader = new Shader("Assets/Shaders/Default.vert", "Assets/Shaders/Default.frag");

		TextureManager::add(new Texture("test00", "tex1.jpg"));
		TextureManager::add(new Texture("test01", "tex2.png"));
		TextureManager::add(new Texture("test02", "tex3.png"));
		TextureManager::add(new Texture("test03", "tex4.png"));

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
				Sprite* iter = new Sprite(glm::vec3(i * 40, j * 40, 0), glm::vec2(40, 40), TextureManager::get("test0" + std::to_string(rand() % 4)));
				m_Renderables.push_back(iter);
			}
		}

		GLint texIDs[] =
		{
			0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
			20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			30, 31
		};

		shader->Enable();
		shader->setUniform1iv("textures", texIDs, 32);

		graphics::FontManager::add(new graphics::Font("testFont", "arial.ttf", 24));
		Label* iter = new Label("FPS test", glm::vec3(20, 670, 0), graphics::FontManager::get("testFont"), 0xffffffff);
		m_Renderables.push_back(iter);

		audio::SoundManager::getMusic("bgm")->play();
		audio::SoundManager::getSound("eff")->loop();
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