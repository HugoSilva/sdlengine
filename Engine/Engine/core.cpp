#include "core.h"

#include <filesystem>
#include <functional>
#include <imgui.h>

#include <ecs/scene_io.h>
#include <render/imgui_layer.h>

#ifdef EMSCRIPTEN
static void DispatchLoop(void* fp) {
  std::function<void()>* func = (std::function<void()>*)fp;
  (*func)();
}
#endif

bool Core::m_running = true;
rse::Renderer2D* Core::m_renderer;
rse::Window* Core::m_window;
rse::Project Core::m_project;

namespace {
// The engine's own copy of a resource, next to the binary.
std::string EngineResourcePath(const std::string& file_name) {
  return (std::filesystem::path("Resources") / file_name).string();
}
}  // namespace

Core::Core(const std::filesystem::path& project_file) : m_frames_per_second(0) {
  LoadProject(project_file);

  const bool project_loaded = m_project.IsAttached();

  m_window =
      new rse::Window(project_loaded ? m_project.window.title.c_str() : "RockSlide Engine",
                      project_loaded ? m_project.window.width : 1280, project_loaded ? m_project.window.height : 720);
  m_window->Init();

  // Scene paths resolve against the project directory; without a project they
  // resolve against the working directory, as they always did.
  SceneManager::Init(project_loaded ? m_project.Directory() : std::filesystem::current_path());

  rse::SoundManager::Init();
  rse::FontManager::Init();
  rse::TextureManager::Init();
  rse::InputManager::Init();
  PhysicsManager::Init();

  const std::string vertex_shader = ResolveResource("Default.vert");
  const std::string fragment_shader = ResolveResource("Default.frag");
  rse::Shader* default_shader = new rse::Shader(vertex_shader.c_str(), fragment_shader.c_str());
  m_renderer = new rse::OpenGLRenderer(m_window->GetWindow(), default_shader);

  rse::ImGuiLayer::Init(m_window->GetWindow(), m_window->GetContext());

  rse::ECSManager::Init();

  if (project_loaded) {
    std::string error;

    if (!SceneManager::Open(m_project.startup_scene, &error)) {
      Logger::Error("Failed to open the startup scene: " + error);
    }
  }
}

void Core::LoadProject(const std::filesystem::path& project_file) {
  rse::Project project;
  std::string error;

  if (!rse::Project::Load(project_file, project, &error)) {
    // Running with no project at all is a supported way to start the binaries;
    // a project that exists but cannot be read is not the same thing.
    if (std::filesystem::exists(project_file)) {
      Logger::Error("Failed to open the project: " + error);
      Logger::Log("Running without a project.");
    } else {
      Logger::Log("No project at " + project_file.string() + "; running without one.");
    }
    return;
  }

  m_project = project;
  Logger::Log("Opened project " + m_project.name + " (" + m_project.File().string() + ")");
}

bool Core::OpenProject(const std::filesystem::path& project_file, std::string* error) {
  rse::Project project;

  if (!rse::Project::Load(project_file, project, error)) {
    return false;
  }

  m_project = project;
  SceneManager::Init(m_project.Directory());

  if (m_window != nullptr) {
    m_window->SetTitle(m_project.window.title);
    SDL_SetWindowSize(m_window->GetWindow(), m_project.window.width, m_project.window.height);
  }

  if (!SceneManager::Open(m_project.startup_scene, error)) {
    return false;
  }

  Logger::Log("Opened project " + m_project.name + " (" + m_project.File().string() + ")");
  return true;
}

bool Core::SaveProject(std::string* error) {
  if (!m_project.IsAttached()) {
    if (error != nullptr) {
      *error = "no project is open";
    }
    return false;
  }

  if (!SceneManager::Save(error)) {
    return false;
  }

  return m_project.Save(error);
}

std::string Core::ResolveResource(const std::string& name) {
  if (m_project.IsAttached()) {
    const std::filesystem::path candidate = m_project.Resolve(m_project.asset_root + "/" + name);

    if (std::filesystem::exists(candidate)) {
      return candidate.string();
    }
  }

  return EngineResourcePath(name);
}

Core::~Core() {
  SceneManager::Clean();

  // Components borrow what the managers own -- a sprite its texture, an audio
  // component its sound -- so the registry is emptied while both are still
  // alive. Destroying an entity after its manager would leave it holding a
  // pointer to something already deleted, and destroying it before the window
  // goes means the textures still have a context to be freed in.
  rse::SceneIO::Clear();
  rse::ECSManager::Clean();

  rse::SoundManager::Clean();
  rse::FontManager::Clean();
  rse::TextureManager::Clean();
  PhysicsManager::Clean();

  // Must happen while the GL context and the SDL window are still alive.
  rse::ImGuiLayer::Shutdown();

  delete m_window;
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
  Update(m_delta_time);

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
  Render();

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