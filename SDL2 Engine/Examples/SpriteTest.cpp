#include "SpriteTest.h"

namespace examples {

	SpriteTest::SpriteTest(SDL_Window* win)
	{
		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
			return;

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

		camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));

		wave = Mix_LoadWAV("effect.wav");
		if (wave == NULL)
			return;

		music = Mix_LoadMUS("background.ogg");
		if (music == NULL)
			return;

		if (Mix_PlayChannel(-1, wave, 0) == -1)
			return;

		if (Mix_PlayMusic(music, -1) == -1)
			return;

		//while (Mix_PlayingMusic());
	}

	SpriteTest::~SpriteTest()
	{
		// clean up our resources
		Mix_FreeChunk(wave);
		Mix_FreeMusic(music);

		// quit SDL_mixer
		Mix_CloseAudio();
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