#include "Scene01.h"

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

		//m_BaseLayer = new graphics::Layer(new graphics::SDLRenderer(win), shader);
		m_BaseLayer = new graphics::Layer(new graphics::OpenGLRenderer(win), shader);

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
				m_BaseLayer->add(iter);
			}
		}

		graphics::FontManager::add(new graphics::Font("testFont", "arial.ttf", 24));
		Label* iter = new Label("FPS test", glm::vec3(20, 670, 0), graphics::FontManager::get("testFont"), 0xffffffff);
		m_BaseLayer->add(iter);

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
		m_BaseLayer->render();
	}
}