#pragma once

#include <Core.h>
#include <glm\glm.hpp>
#include "Scene01.h"

using namespace graphics;
using namespace audio;

class Game : public Core
{

public:
	Game()
	{

	}

	~Game()
	{
		delete m_Layer;
	}

	void init() override
	{
		m_Window = createWindow("RockSlide Engine", 1280, 720);
		LoadScene(new SpriteTest(m_Window->GetWindow()));
		//FontManager::add(new Font("Arial", "Resources/arial.ttf", 24));

		//m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");

		//m_Layer = new Layer(new OpenGLRenderer(m_Window->GetWindow()), m_Shader);

		//TextureManager::add(new Texture("test00", "Resources/tex1.jpg"));

		//m_Sprite = new Sprite(glm::vec3(40, 40, 0), glm::vec2(40, 40), TextureManager::get("test00"));

		//m_Layer->add(m_Sprite);

		//m_Fps = new Label("FPS test", glm::vec3(20, 670, 0), graphics::FontManager::get("Arial"), 0xffffffff);
		//m_Layer->add(m_Fps);

		//SoundManager::add(new Sound("eff", "Resources/effect.wav"));
		//SoundManager::add(new Music("bgm", "Resources/background.ogg"));
	}

	void render() override
	{
	}

private:
	Window* m_Window;
	Layer* m_Layer;
	Label* m_Fps;
	Sprite* m_Sprite;
	Shader* m_Shader;
};

int main(int argc, char **argv)
{
	Game game;
	game.start();
	return 0;
}