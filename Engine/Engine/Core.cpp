#include "Core.h"

#include <functional>
#include <imgui.h>

#include "Render/ImGuiLayer.hpp"

#ifdef EMSCRIPTEN
static void DispatchLoop(void* fp) {
  std::function<void()>* func = (std::function<void()>*)fp;
  (*func)();
}
#endif

bool Core::m_running = true;
rse::Renderer2D* Core::m_renderer;
rse::Window* Core::m_window;

Core::Core() : m_frames_per_second(0) {
  m_window = new rse::Window("RockSlide Engine", 1280, 720);
  m_window->Init();

  rse::SoundManager::Init();
  rse::FontManager::Init();
  rse::TextureManager::Init();
  rse::InputManager::Init();
  PhysicsManager::Init();

  rse::Shader* default_shader = new rse::Shader("Resources/Default.vert", "Resources/Default.frag");
  m_renderer = new rse::OpenGLRenderer(m_window->GetWindow(), default_shader);

  rse::ImGuiLayer::Init(m_window->GetWindow(), m_window->GetContext());

  rse::ECSManager::Init();
}

Core::~Core() {
  rse::SoundManager::Clean();
  rse::FontManager::Clean();
  rse::TextureManager::Clean();
  PhysicsManager::Clean();
  rse::ECSManager::Clean();

  // Must happen while the GL context and the SDL window are still alive.
  rse::ImGuiLayer::Shutdown();

  delete m_window;
}

rse::Window* Core::createWindow(const char* name, int width, int height) {
  // m_Window = new graphics::Window(name, width, height);
  // m_Window->Init();
  // return m_Window;
  return nullptr;
}

void Core::Start() {
  Init();
  try {
#ifdef EMSCRIPTEN
    std::function<void()> fGameLoop = [&]() {
#else
    while (m_window->GetRunning()) {
#endif
      Run();

#ifdef EMSCRIPTEN
    };
    emscripten_set_main_loop_arg(DispatchLoop, &fGameLoop, 0, 1);
#else
    }
    m_running = false;
#endif
  } catch (...) {
    Logger::Error("Failed to start the application");
  }
}

void Core::Run() {
  m_now = SDL_GetTicks();
  m_delta_time = (m_now - m_last) / 1000.0f;
  m_last = m_now;

  rse::InputManager::Update();

  // The ImGui frame has to be open before anything submits widgets, and
  // closed after the scene has been drawn, so that the UI ends up on top.
  rse::ImGuiLayer::BeginFrame();

  // TODO review scene manager logic now that we are using ECS
  SceneManager::Update(m_delta_time);

  // The UI is built before the scene is drawn: panels may need to know their
  // own size in order to size a render target (the viewport does this).
  rse::ECSManager::UpdateUi();

  // The editor draws the scene into its viewport panel's framebuffer; a game
  // returns nullptr here and draws straight to the window.
  rse::Framebuffer* scene_target = GetSceneTarget();
  if (scene_target != nullptr) {
    scene_target->Bind();
  } else {
    m_window->BindDefaultFramebuffer();
  }

  // The scene's shader uniforms are uploaded *after* the target is bound and
  // after the UI phase. Both halves matter: binding the target is what sets
  // the viewport the camera sizes its projection to, and the UI is where the
  // viewport panel pans and zooms. Uploading them any earlier is what made
  // every pan and zoom lag the scene by exactly one frame.
  SceneManager::Render();  // Only for shader

  m_renderer->Begin();
  // TODO need to split this up for normal systems and render systems
  rse::ECSManager::Update(m_delta_time);
  m_renderer->End();
  m_renderer->Flush();

  if (scene_target != nullptr) {
    rse::Framebuffer::Unbind();
  }

  rse::ImGuiLayer::EndFrame();

  m_delta_accumulator += m_delta_time;
  m_frames++;

  if (m_delta_accumulator > 1.0f) {
    m_frames_per_second = m_frames;
    m_frames = 0;
    m_delta_accumulator = 0.f;
    Tick();
  }
}