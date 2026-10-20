#include "editor_app.h"

#include <editor_manager.h>
#include <imgui.h>
#include <project_commands.h>

#include <layout/editor_layout.h>
#include <panels/console_panel.h>
#include <panels/content_browser_panel.h>
#include <panels/debug_panel.h>
#include <panels/hierarchy_panel.h>
#include <panels/inspector_panel.h>
#include <panels/viewport_panel.h>
#include <render/grid_render_system.h>
#include <systems/render_system.h>

EditorApp::EditorApp(const std::filesystem::path& project_file) : Core(project_file) {}

void EditorApp::Init() {
  using namespace rse;

  const std::string vertex_shader = Core::ResolveResource("Default.vert");
  const std::string fragment_shader = Core::ResolveResource("Default.frag");
  m_shader = new Shader(vertex_shader.c_str(), fragment_shader.c_str());

  TextureManager::Add(new Texture("test00", Core::ResolveResource("tex3.png")));
  SoundManager::Add(new Sound("eff", Core::ResolveResource("effect.wav")));

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

  EditorManager::Init();

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
    const bool control =
        rse::InputManager::IsKeyPressed(SDL_SCANCODE_LCTRL) || rse::InputManager::IsKeyPressed(SDL_SCANCODE_RCTRL);

    if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F1) ||
        (control && rse::InputManager::IsKeyPressed(SDL_SCANCODE_S))) {
      ::SaveProject();
    }

    if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F2)) {
      ::ReloadScene();
    }
  }

  UpdateTitle();
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
