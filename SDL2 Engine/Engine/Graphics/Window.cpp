#include "Window.h"

namespace graphics {

	Window::Window(const char *title, int width, int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{
		running = true;
	}

	Window::~Window()
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	bool Window::Init()
	{
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(0, &mode);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		window = SDL_CreateWindow(m_Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
		if (window == nullptr) {
			std::cout << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
			SDL_Quit();
		}

		context = SDL_GL_CreateContext(window);

		glewExperimental = GL_TRUE;
		GLenum result = glewInit();
		if (result != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW: " << glewGetErrorString(result) << std::endl;
		}

		SDL_ShowCursor(SDL_DISABLE);
		SDL_SetWindowGrab(window, SDL_TRUE);
		//TODO verify the SDL_GL_SetSwapInterval setting
		SDL_GL_SetSwapInterval(1);

		glViewport(0, 0, m_Width, m_Height);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		m_Input = new Input();

		std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

		return true;
	}

	void Window::Run()
	{
		now = SDL_GetTicks();
		deltaTime = (now - last) / 1000.0f;
		last = now;
		this->HandleEvents();
		m_Scene->Update(deltaTime);

		//glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Scene->Render();

		//SDL_GL_SwapWindow(window);

		deltaAccumulator += deltaTime;
		frames++;

		if (deltaAccumulator > 1.0f)
		{
			printf("%d FPS\n", frames);
			frames = 0;
			deltaAccumulator = 0.f;
		}
	}

	void Window::HandleEvents()
	{
		if (SDL_PollEvent(&windowEvent)) {
			switch (windowEvent.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				m_Input->HandleKeyboard(windowEvent.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				m_Input->HandleMouseButton(windowEvent.button);
			case SDL_MOUSEMOTION:
				m_Input->HandleMouseMotion(windowEvent.motion);
				break;
			}
		}
	}

	void Window::LoadScene(Scene* scene)
	{
		m_Scene = scene;
	}

	bool Window::GetRunning()
	{
		return running;
	}
}