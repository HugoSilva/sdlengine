#include "Window.hpp"

#include "../Input/InputManager.hpp"
#include "../Utils/Logger.hpp"

#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

namespace rse {
Window::Window(const char* title, int width, int height) : m_Title(title), m_Width(width), m_Height(height) {}

Window::~Window() {
  // ImGui is owned by rse::ImGuiLayer now (see ImGuiLayer::shutdown()).
  SDL_DestroyWindow(m_Window);
  SDL_Quit();
}

bool Window::init() {
  // SDL3 removed SDL_INIT_EVERYTHING; ask for exactly what the engine uses.
  // VIDEO/AUDIO/GAMEPAD all imply SDL_INIT_EVENTS.
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    Logger::error(std::string("Failed to initialize SDL: ") + SDL_GetError());
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // SDL3 dropped the x/y window position arguments (SDL_WINDOWPOS_CENTERED);
  // SDL now places the window, or you can SDL_SetWindowPosition() later.
  m_Window = SDL_CreateWindow(m_Title, m_Width, m_Height,
                              SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (m_Window == nullptr) {
    Logger::error(std::string("Failed to create SDL Window: ") + SDL_GetError());
    SDL_Quit();
    return false;
  }

  m_Context = SDL_GL_CreateContext(m_Window);

#ifndef EMSCRIPTEN
  glewExperimental = GL_TRUE;
  GLenum result = glewInit();
  if (result != GLEW_OK) {
    Logger::error(std::string("Failed to initialize GLEW: ") +
                  reinterpret_cast<const char*>(glewGetErrorString(result)));
  }
#endif

  // TODO show cursor and window focus
  // SDL_HideCursor();
  // TODO Fix this, I had to Disabled VSync to test performance in release mode
  SDL_GL_SetSwapInterval(0);

  glViewport(0, 0, m_Width, m_Height);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glEnable(GL_BLEND);
  // Blending keeps the destination alpha at 1: the colour channels blend
  // normally, but alpha is written as GL_ONE so partial-alpha geometry (the
  // editor grid, a translucent sprite) cannot leave a render target with a
  // see-through alpha. Anything that draws that target a second time --
  // the editor viewport hands its framebuffer to ImGui -- blends by alpha,
  // so a leaking target would be darkened by whatever sits behind it.
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  const GLubyte* version = glGetString(GL_VERSION);
  Logger::log(std::string("OpenGL ") + (version != nullptr ? reinterpret_cast<const char*>(version) : "unknown"));

  // ImGui is initialised separately once the window exists -- see
  // rse::ImGuiLayer::init(), called from Core.
  return true;
}

bool Window::getRunning() {
  // TODO I need to rethink the quit logic for the window
  return !InputManager::IsQuitRequested();
}

void Window::bindDefaultFramebuffer() {
  int width = 0;
  int height = 0;
  SDL_GetWindowSizeInPixels(m_Window, &width, &height);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
}
}  // namespace rse
