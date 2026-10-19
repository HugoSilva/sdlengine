#include "setup_view.h"

#include <editor_manager.h>

#include <layout/editor_layout.h>
#include <panels/console_panel.h>
#include <panels/content_browser_panel.h>
#include <panels/debug_panel.h>
#include <panels/hierarchy_panel.h>
#include <panels/inspector_panel.h>
#include <panels/viewport_panel.h>
#include <render/grid_render_system.h>

SetupView::SetupView(std::string name, SDL_Window* win) {
  if (File::Exists("default.cfg")) {
    File::Read("default.cfg");
  } else {
    File::Write("default.cfg", "definitions should be passed here");
  }

  using namespace rse;

  m_shader = new Shader("Resources/Default.vert", "Resources/Default.frag");

  // TODO I need to add a function to load resources in the project
  TextureManager::Add(new Texture("test00", "Resources/tex3.png"));
  SoundManager::Add(new Sound("eff", "Resources/effect.wav"));

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
  rse::ECSManager::AddSystem(m_render);

  // The grid is submitted into the scene pass before the sprites are, which is
  // what puts it underneath them.
  rse::ECSManager::AddSystem(new GridRenderSystem(Core::GetRenderer(), m_camera), rse::SystemPhase::kPreScene);

  // New Entities code
  // for (int i = 0; i < 5000; i++)
  //{
  //	rse::PositionComponent position;
  //	position.x = rand() % (1200 - 40 + 1) + 10;
  //	position.y = rand() % (720 - 40 + 1) + 10;
  //	position.z = 0.f;

  //	rse::SpriteComponent sprite;
  //	//sprite.Sprite = new );

  //	uint32_t entity3 = rse::ECSManager::createEntity();
  //	rse::ECSManager::addComponent<TransformComponent>(entity3, glm::vec3(position.x, position.y, position.z),
  // glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)); 	rse::ECSManager::addComponent<SpriteComponent>(entity3,
  // new Sprite(glm::vec3(position.x, position.y, position.z), glm::vec2(40, 40), TextureManager::get("test00")));
  //}

  m_name = name;

  EditorManager::Init();
}

void SetupView::Update(float delta_time) {
  // SceneManager::change("SelectProject");

  if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F1)) rse::ECSManager::Save();
  if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F2)) rse::ECSManager::Load();
}

void SetupView::Render() {
  m_shader->Enable();
  m_camera->Apply(m_shader);
}