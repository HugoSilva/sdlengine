#include "Core.h"

Core::Core()
	: m_FramesPerSecond(0), m_UpdatesPerSecond(0)
{

}

Core::~Core()
{
	delete m_Timer;
	delete m_Window;
}

graphics::Window* Core::createWindow(const char *name, int width, int height)
{
	m_Window = new graphics::Window(name, width, height);
	return m_Window;
}

void Core::start()
{
	init();
	run();
}

void Core::run()
{
	m_Timer = new Timer();
	float timer = 0.0f;
	float updateTimer = 0.0f;
	float updateTick = 1.0f / 60.0f;
	unsigned int frames = 0;
	unsigned int updates = 0;

	while (!m_Window->closed())
	{
		m_Window->clear();
		if (m_Timer->elapsed() - updateTimer > updateTick)
		{
			m_Window->updateInput();
			update();
			updates++;
			updateTimer += updateTick;
		}
		render();
		frames++;
		m_Window->update();
		if (m_Timer->elapsed() - timer > 1.0f)
		{
			timer += 1.0f;
			m_FramesPerSecond = frames;
			m_UpdatesPerSecond = updates;
			frames = 0;
			updates = 0;
			tick();
		}
	}
}