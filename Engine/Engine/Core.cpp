#include "Core.h"
#include <functional>

#ifdef EMSCRIPTEN
static void DispatchLoop(void* fp)
{
	std::function<void()>* func = (std::function<void()>*)fp;
	(*func)();
}
#endif

Core::Core() : m_FramesPerSecond(0)
{

}

Core::~Core()
{
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
#ifdef EMSCRIPTEN
		std::function<void()> fGameLoop = [&]() {
#else
		while (m_Window->GetRunning())
		{
#endif
			m_Window->Run();
			run();

#ifdef EMSCRIPTEN
		};
		emscripten_set_main_loop_arg(DispatchLoop, &fGameLoop, 0, 1);
#else
		}
#endif
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