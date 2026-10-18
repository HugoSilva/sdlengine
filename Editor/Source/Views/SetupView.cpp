#include "SetupView.h"

#include "../Layout/EditorLayout.hpp"
#include "../Panels/ConsolePanel.hpp"
#include "../Panels/ContentBrowserPanel.hpp"
#include "../Panels/DebugPanel.hpp"
#include "../Panels/HierarchyPanel.hpp"
#include "../Panels/InspectorPanel.hpp"
#include "../Panels/ViewportPanel.hpp"
#include "../EditorManager.h"

SetupView::SetupView(std::string name, SDL_Window* win)
{
	if (File::exists("default.cfg"))
	{
		File::read("default.cfg");
	}
	else
	{
		File::write("default.cfg", "definitions should be passed here");
	}

	using namespace rse;

	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");

	//TODO I need to add a function to load resources in the project
	TextureManager::add(new Texture("test00", "Resources/tex3.png"));
	SoundManager::add(new Sound("eff", "Resources/effect.wav"));

	// The viewport gets the editor's own camera, not the engine's game camera:
	// the engine camera keeps its WASD/mouse-look path and the editor camera
	// owns pan/zoom and the projection the viewport draws with.
	m_Camera = new EditorCamera();

	m_Layout = new EditorLayout();
	m_Layout->addPanel(std::make_unique<HierarchyPanel>());
	m_Layout->addPanel(std::make_unique<ContentBrowserPanel>());
	m_Layout->addPanel(std::make_unique<DebugPanel>());
	m_Layout->addPanel(std::make_unique<ViewportPanel>(m_Camera));
	m_Layout->addPanel(std::make_unique<InspectorPanel>());
	m_Layout->addPanel(std::make_unique<ConsolePanel>());

	m_render = new RenderSystem(m_Layout);
	rse::ECSManager::addSystem(m_render);

	//New Entities code
	//for (int i = 0; i < 5000; i++)
	//{
	//	rse::PositionComponent position;
	//	position.x = rand() % (1200 - 40 + 1) + 10;
	//	position.y = rand() % (720 - 40 + 1) + 10;
	//	position.z = 0.f;

	//	rse::SpriteComponent sprite;
	//	//sprite.Sprite = new );

	//	uint32_t entity3 = rse::ECSManager::createEntity();
	//	rse::ECSManager::addComponent<TransformComponent>(entity3, glm::vec3(position.x, position.y, position.z), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f));
	//	rse::ECSManager::addComponent<SpriteComponent>(entity3, new Sprite(glm::vec3(position.x, position.y, position.z), glm::vec2(40, 40), TextureManager::get("test00")));
	//}

	m_Name = name;

	EditorManager::init();
}

void SetupView::Update(float deltaTime)
{
    //SceneManager::change("SelectProject");

	if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F1))
		rse::ECSManager::save();
	if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F2))
		rse::ECSManager::load();
}

void SetupView::Render()
{
	m_Shader->enable();
	m_Camera->apply(m_Shader);
}