#include "editor_app.h"

#include <editor_manager.h>
#include <imgui.h>
#include <project_commands.h>

#include <input/input_map.h>
#include <layout/editor_layout.h>
#include <panels/console_panel.h>
#include <panels/content_browser_panel.h>
#include <panels/debug_panel.h>
#include <panels/hierarchy_panel.h>
#include <panels/inspector_panel.h>
#include <panels/viewport_panel.h>
#include <render/grid_render_system.h>
#include <systems/render_system.h>

namespace {
// Ctrl+S is a hard fallback, and so is F1: a project that rebinds its actions --
// or binds editor.save to something it cannot press -- must not lock the editor
// out of saving. File scope rather than the function's, so the two are built
// once and are read-only everywhere they are used.
const rse::Binding kSaveFallback{SDL_SCANCODE_S, true, false, false};
const rse::Binding kLegacySaveFallback{SDL_SCANCODE_F1, false, false, false};
}  // namespace

EditorApp::EditorApp(const std::filesystem::path& project_file) : Core(project_file) {}

void EditorApp::Init() {
  using namespace rse;

  // Before the panels exist: the settings are what decide their initial
  // visibility, and EditorLayout::AddPanel reads them.
  EditorManager::Init();

  // The mix the project was saved with, so the editor hears what the game will
  // before a slider is touched.
  SoundManager::SetVolumes(Core::GetProject().audio.master, Core::GetProject().audio.music,
                           Core::GetProject().audio.sfx);

  const std::string vertex_shader = Core::ResolveResource("Default.vert");
  const std::string fragment_shader = Core::ResolveResource("Default.frag");
  m_shader = new Shader(vertex_shader.c_str(), fragment_shader.c_str());

  SoundManager::Add(std::make_unique<Sound>("eff", Core::ResolveResource("effect.wav")));

  // The viewport gets the editor's own camera, not the engine's game camera:
  // the engine camera keeps its WASD/mouse-look path and the editor camera
  // owns pan/zoom and the projection the viewport draws with.
  m_camera = new EditorCamera();

  m_layout = new EditorLayout();
  m_layout->AddPanel(std::make_unique<HierarchyPanel>());
  m_layout->AddPanel(std::make_unique<ContentBrowserPanel>());
  m_layout->AddPanel(std::make_unique<DebugPanel>());
  m_layout->AddPanel(std::make_unique<ViewportPanel>(m_camera));
  m_layout->AddPanel(std::make_unique<InspectorPanel>());
  m_layout->AddPanel(std::make_unique<ConsolePanel>());

  m_render = new RenderSystem(m_layout);
  ECSManager::AddSystem(m_render);

  // The grid is submitted into the scene pass before the sprites are, which is
  // what puts it underneath them.
  ECSManager::AddSystem(new GridRenderSystem(Core::GetRenderer(), m_camera), rse::SystemPhase::kPreScene);

#ifdef EMSCRIPTEN
#else
  ThreadManager::Init();
  ThreadManager::Start();
#endif
}

void EditorApp::Update(float delta_time) {
  // Shortcuts stay out of the way of text fields: "S" is a letter before it is
  // a shortcut, and typing a path must not save over anything.
  if (!ImGui::GetIO().WantTextInput) {
    // The bindings come first so a project can add a key, not remove the only
    // one -- see kSaveFallback.
    if (rse::InputMap::IsActionPressed("editor.save") || rse::InputMap::IsPressed(kSaveFallback) ||
        rse::InputMap::IsPressed(kLegacySaveFallback)) {
      ::SaveProject();
    }

    if (rse::InputMap::IsActionPressed("editor.reload")) {
      ::ReloadScene();
    }
  }

  // A project or scene opened this frame is remembered now, not only on exit:
  // a crash should not lose the project the user was in.
  RememberSession();

  UpdateTitle();
}

void EditorApp::RememberSession() {
  EditorSettings& settings = EditorManager::Settings();

  const rse::Project& project = Core::GetProject();
  const std::string last_scene = SceneManager::GetActiveScene().string();
  const std::vector<std::string> recent_projects = settings.recent_projects;

  // The open project is a recent one wherever it came from: a dialog, the
  // typed-path prompt, the command line, or project.xml next to the binary.
  // RememberRecentProject is idempotent, so doing it every frame costs a
  // comparison; the file is written only when the list or the scene changed.
  if (project.IsAttached()) {
    settings.RememberRecentProject(project.File().string());
  }

  if (settings.last_scene == last_scene && settings.recent_projects == recent_projects) {
    return;
  }

  settings.last_scene = last_scene;
  settings.Save();
}

void EditorApp::Shutdown() {
  EditorSettings& settings = EditorManager::Settings();

  // The panels are the truth about what is on screen; the settings are what the
  // next run reads before it builds them.
  if (m_layout != nullptr) {
    m_layout->SyncVisibility(settings);
  }

  RememberSession();
  settings.Save();
}

void EditorApp::UpdateTitle() {
  const std::string scene = SceneManager::GetActiveSceneName();
  const rse::Project& project = Core::GetProject();

  std::string title;

  if (SceneManager::IsDirty()) {
    title += "* ";
  }

  if (!scene.empty()) {
    title += scene + " - ";
  }

  if (project.IsAttached()) {
    title += project.name + " - ";
  }

  title += "RockSlide Engine";

  if (title == m_title) {
    return;
  }

  m_title = title;
  Core::GetWindow()->SetTitle(m_title);
}

void EditorApp::Render() {
  m_shader->Enable();
  m_camera->Apply(m_shader);
}

rse::Framebuffer* EditorApp::GetSceneTarget() { return EditorManager::GetSceneTarget(); }
