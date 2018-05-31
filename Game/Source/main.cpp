#pragma once

#include <Core.h>
#include <glm\glm.hpp>
#include "Scene01.h"

class Game : public Core
{

public:
	Game()
	{

	}

	~Game()
	{
	}

	void init() override
	{
		m_Window = createWindow("RockSlide Engine", 1280, 720);
		AddScene(new SpriteTest(m_Window->GetWindow()));
		ThreadManager::Init();
	}

	void render() override
	{
		
	}

private:
	graphics::Window * m_Window;
};

int main(int argc, char **argv)
{
	Game game;
	game.start();
	return EXIT_SUCCESS;
}