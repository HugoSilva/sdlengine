#include "Core.h"

bool Core::m_Running = true;

Core::Core() : m_FramesPerSecond(0)
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
	try
	{
		while (m_Window->GetRunning())
		{
			run();
		}
		m_Running = false;
	}
	catch (...)
	{
		Logger::error("Failed to start the application");
	}
}

void Core::run()
{
	now = SDL_GetTicks();
	deltaTime = (now - last) / 1000.0f;
	last = now;

	IO::InputManager::Update();

	m_Scenes.at(m_ActiveScene)->Update(deltaTime);

	m_Scenes.at(m_ActiveScene)->Render();

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

bool Core::AddScene(Scene* scene)
{
	if (true)
	{
		m_Scenes.push_back(scene);
		m_ActiveScene = m_Scenes.size()-1;
		return true;
	}
	return false;
}

bool Core::ChangeScene(Scene* newScene)
{
	for (unsigned int i = 0; i < m_Scenes.size(); i++)
	{
		if (m_Scenes.at(i) == newScene)
		{
			m_ActiveScene = i;
			return true;
		}
	}
	return false;
}