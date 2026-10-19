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

bool Core::m_Running = true;
rse::Renderer2D* Core::m_Renderer;
rse::Window* Core::m_Window;

Core::Core() : m_FramesPerSecond(0) {
  m_Window = new rse::Window("RockSlide Engine", 1280, 720);
  m_Window->Init();

  rse::SoundManager::Init();
  rse::FontManager::Init();
  rse::TextureManager::Init();
  rse::InputManager::Init();
  PhysicsManager::Init();

  rse::Shader* defaultShader = new rse::Shader("Resources/Default.vert", "Resources/Default.frag");
  m_Renderer = new rse::OpenGLRenderer(m_Window->GetWindow(), defaultShader);

  rse::ImGuiLayer::Init(m_Window->GetWindow(), m_Window->GetContext());

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

  delete m_Window;
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
    while (m_Window->GetRunning()) {
#endif
      Run();

#ifdef EMSCRIPTEN
    };
    emscripten_set_main_loop_arg(DispatchLoop, &fGameLoop, 0, 1);
#else
    }
    m_Running = false;
#endif
  } catch (...) {
    Logger::Error("Failed to start the application");
  }
}

void Core::Run() {
  now = SDL_GetTicks();
  deltaTime = (now - last) / 1000.0f;
  last = now;

  rse::InputManager::Update();

  // The ImGui frame has to be open before anything submits widgets, and
  // closed after the scene has been drawn, so that the UI ends up on top.
  rse::ImGuiLayer::BeginFrame();

  // TODO review scene manager logic now that we are using ECS
  SceneManager::Update(deltaTime);

  // The UI is built before the scene is drawn: panels may need to know their
  // own size in order to size a render target (the viewport does this).
  rse::ECSManager::UpdateUi();

  // The editor draws the scene into its viewport panel's framebuffer; a game
  // returns nullptr here and draws straight to the window.
  rse::Framebuffer* sceneTarget = GetSceneTarget();
  if (sceneTarget != nullptr) {
    sceneTarget->Bind();
  } else {
    m_Window->BindDefaultFramebuffer();
  }

  // The scene's shader uniforms are uploaded *after* the target is bound and
  // after the UI phase. Both halves matter: binding the target is what sets
  // the viewport the camera sizes its projection to, and the UI is where the
  // viewport panel pans and zooms. Uploading them any earlier is what made
  // every pan and zoom lag the scene by exactly one frame.
  SceneManager::Render();  // Only for shader

  m_Renderer->Begin();
  // TODO need to split this up for normal systems and render systems
  rse::ECSManager::Update(deltaTime);
  m_Renderer->End();
  m_Renderer->Flush();

  if (sceneTarget != nullptr) {
    rse::Framebuffer::Unbind();
  }

  rse::ImGuiLayer::EndFrame();

  deltaAccumulator += deltaTime;
  frames++;

  if (deltaAccumulator > 1.0f) {
    m_FramesPerSecond = frames;
    frames = 0;
    deltaAccumulator = 0.f;
    Tick();
  }
}