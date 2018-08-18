#include "SetupView.h"

#include "../Components/EntityViewerComponent.hpp"
#include "../Components/PropertyComponent.hpp"
#include "../Components/MenuBarComponent.hpp"
#include "../../EditorManager.h"

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

	using namespace graphics;

	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");
	m_Layer = new Layer(new OpenGLRenderer(win), m_Shader);

	//ImguiMenuBar* menubar = new ImguiMenuBar();
	//m_Layer->add(menubar);
	m_render = new RenderSystem(Core::getRenderer());
	ecs::ECSManager::addSystem(m_render);

	uint32_t entity = ecs::ECSManager::createEntity();
	ecs::ECSManager::addComponent<DebugComponent>(entity, new MenuBarComponent());

	uint32_t entity1 = ecs::ECSManager::createEntity();
	ecs::ECSManager::addComponent<DebugComponent>(entity1, new EntityViewerComponent());

	uint32_t entity2 = ecs::ECSManager::createEntity();
	ecs::ECSManager::addComponent<DebugComponent>(entity2, new PropertyComponent());

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Name = name;

	EditorManager::init();
}

void SetupView::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);
	//SceneManager::change("SelectProject");
}

void SetupView::Render()
{
	m_Shader->Enable();
	m_Camera->Render(m_Shader);
	//m_Layer->render();
}