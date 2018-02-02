#include "Window.h"

#include <GL/glew.h>

namespace graphics {

	Window::Window(const char *title, int width, int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{
		running = true;
	}

	Window::~Window()
	{
		audio::SoundManager::clean();
		graphics::FontManager::clean();
		TextureManager::clean();

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

		window = SDL_CreateWindow(m_Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

		audio::SoundManager::init();
		graphics::FontManager::init();
		TextureManager::init();

		//TODO show cursor and window focus 
		//SDL_ShowCursor(SDL_DISABLE);
		//SDL_SetWindowGrab(window, SDL_TRUE);
		//TODO Fix this, I had to Disabled VSync to test performance in release mode
		SDL_GL_SetSwapInterval(0);

		glViewport(0, 0, m_Width, m_Height);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL); 
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_Input = new Input();

		std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

		return true;
	}

	void Window::Run()
	{
		this->HandleEvents();
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

	bool Window::GetRunning()
	{
		return running;
	}
}