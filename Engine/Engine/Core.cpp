#include "Core.h"

Core::Core()
	: m_FramesPerSecond(0)
{

}

Core::~Core()
{
	//delete m_Timer;
	delete m_Window;
}

graphics::Window* Core::createWindow(const char *name, int width, int height)
{
	m_Window = new graphics::Window(name, width, height);
	m_Window->Init();
	return m_Window;
}

void Core::start()
{
	init();
	run();
}

void Core::run()
{
	now = SDL_GetTicks();
	deltaTime = (now - last) / 1000.0f;
	last = now;

	m_Scene->Update(deltaTime);

	m_Scene->Render();

	deltaAccumulator += deltaTime;
	frames++;

	if (deltaAccumulator > 1.0f)
	{
		m_FramesPerSecond = frames;
		frames = 0;
		deltaAccumulator = 0.f;
		tick();
	}
}

void Core::LoadScene(Scene* scene)
{
	m_Scene = scene;
}