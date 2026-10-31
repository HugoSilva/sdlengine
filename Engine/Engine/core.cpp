#include "core.h"

#include <filesystem>
#include <functional>
#include <imgui.h>
#include <utility>

#include <core/asset_registry.h>
#include <core/physics_material_manager.h>
#include <ecs/scene_io.h>
#include <input/input_map.h>
#include <platform/sdl_clock.h>
#include <render/framebuffer.h>
#include <render/imgui_layer.h>
#include <render/opengl_renderer.h>
#include <render/window.h>

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

Core::Core(const std::filesystem::path& project_file)
    : m_clock(std::make_unique<rse::platform::SdlClock>()), m_frames_per_second(0) {
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

  m_renderer = new rse::OpenGLRenderer(m_window->GetWindow());

  // The renderer's constructor makes the window's GL context current. Materials
  // are parsed and compiled after that: a material loaded before there is a
  // context would be cached with no program and never tried again.
  rse::MaterialManager::Init();
  rse::PhysicsMaterialManager::Init();

  rse::ImGuiLayer::Init(m_window->GetWindow(), m_window->GetContext());

  rse::ECSManager::Init();

  if (project_loaded) {
    std::string error;

    if (!SceneManager::Open(m_project.startup_scene, &error)) {
      Logger::Error("Failed to open the startup scene: " + error);
    }
  }
}

void Core::ApplyProjectInput() {
  std::vector<std::pair<std::string, std::string>> overrides;
  overrides.reserve(m_project.input.size());

  for (const rse::Project::InputBinding& binding : m_project.input) {
    overrides.emplace_back(binding.action, binding.key);
  }

  rse::InputMap::SetOverrides(overrides);
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

  // The manifest's bindings are part of the project, like its window: they are
  // in place before the startup scene opens, so the first frame already
  // answers to them.
  ApplyProjectInput();

  // The asset registry is part of the project: its table is loaded before any
  // scene, because a scene resolves its textures and sounds through it.
  rse::AssetRegistry::Init(m_project.Directory());
  Logger::Log("Opened project " + m_project.name + " (" + m_project.File().string() + ")");
}

bool Core::OpenProject(const std::filesystem::path& project_file, std::string* error) {
  rse::Project project;

  if (!rse::Project::Load(project_file, project, error)) {
    return false;
  }

  m_project = project;
  ApplyProjectInput();
  SceneManager::Init(m_project.Directory());
  rse::AssetRegistry::Init(m_project.Directory());

  // Materials resolve through the registry and compile against the new project,
  // so the old project's programs cannot stay. TextureManager has the same
  // project-lifetime behaviour today; its cache is left alone here.
  rse::MaterialManager::Clean();
  rse::MaterialManager::Init();
  rse::PhysicsMaterialManager::Clean();
  rse::PhysicsMaterialManager::Init();

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

std::string Core::ResolveProjectPath(const std::string& relative) {
  if (m_project.IsAttached()) {
    return m_project.Resolve(relative).string();
  }

  return relative;
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

  rse::AssetRegistry::Clean();
  rse::SoundManager::Clean();
  rse::FontManager::Clean();
  rse::MaterialManager::Clean();
  rse::PhysicsMaterialManager::Clean();
  rse::TextureManager::Clean();
  PhysicsManager::Clean();

  // Must happen while the GL context and the SDL window are still alive.
  rse::ImGuiLayer::Shutdown();

  // The bindings belonged to the project that just closed; a later Core in
  // this process starts from the compiled defaults.
  rse::InputMap::Reset();

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

#ifndef EMSCRIPTEN
  // The window loop has ended, and the scene and the managers are still alive:
  // the last chance for an application to write state that outlives the run.
  Shutdown();
#endif
}

void Core::Run() {
  m_now = m_clock->NowMilliseconds();
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

  // The open scene owns the background colour, so it is applied to the frame
  // that is about to be drawn rather than once at load: a scene opened at
  // runtime changes it on the very next frame.
  m_renderer->SetClearColor(rse::SceneIO::GetCamera().background);

  m_renderer->Begin();
  // TODO need to split this up for normal systems and render systems
  rse::ECSManager::Update(m_delta_time, SceneLogicEnabled());
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