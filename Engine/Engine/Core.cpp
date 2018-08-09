#include "Core.h"
#include <functional>
#include <imgui.h>

#ifdef EMSCRIPTEN
static void DispatchLoop(void* fp)
{
	std::function<void()>* func = (std::function<void()>*)fp;
	(*func)();
}
#endif

bool Core::m_Running = true;
graphics::Renderer2D* Core::m_Renderer;
graphics::Window* Core::m_Window1;

Core::Core() : m_FramesPerSecond(0)
{
	m_Window1 = new graphics::Window("RockSlide Engine", 1280, 720);
	m_Window1->Init();

	m_Renderer = new graphics::OpenGLRenderer(m_Window1->GetWindow());

	ecs::ECSManager::init();
}

Core::~Core()
{
	//delete m_Window;
}

graphics::Window* Core::createWindow(const char *name, int width, int height)
{
	//m_Window = new graphics::Window(name, width, height);
	//m_Window->Init();
	//return m_Window;
	return nullptr;
}

void Core::start()
{
	init();
	try
	{
#ifdef EMSCRIPTEN
		std::function<void()> fGameLoop = [&]() {
#else
		while (m_Window1->GetRunning())
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

	//TODO review scene manager logic now that we are using ECS
	SceneManager::update(deltaTime);

	//TODO shader needs to be moved to the renderable, objects may have different shaders
	SceneManager::render(); // Only for shader
	m_Renderer->begin();
	//TODO need to split this up for normal systems and render systems
	ecs::ECSManager::update(deltaTime);
	m_Renderer->end();
	m_Renderer->flush();

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