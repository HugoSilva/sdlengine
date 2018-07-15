#include "Core.h"
#include <functional>

#ifdef EMSCRIPTEN
static void DispatchLoop(void* fp)
{
	std::function<void()>* func = (std::function<void()>*)fp;
	(*func)();
}
#endif

bool Core::m_Running = true;

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
			run();

#ifdef EMSCRIPTEN
		};
		emscripten_set_main_loop_arg(DispatchLoop, &fGameLoop, 0, 1);
#else
		}
		m_Running = false;
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

	IO::InputManager::Update();
	ecs::ECSManager::update(deltaTime);
	//TODO review scene manager logic now that we are using ECS
	SceneManager::update(deltaTime);
	SceneManager::render();

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