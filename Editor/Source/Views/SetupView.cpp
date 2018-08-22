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

	TextureManager::add(new Texture("test00", "Resources/tex3.png"));

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

	//New Entities code
	//for (int i = 0; i < 5000; i++)
	//{
	//	ecs::PositionComponent position;
	//	position.x = rand() % (1200 - 40 + 1) + 10;
	//	position.y = rand() % (720 - 40 + 1) + 10;
	//	position.z = 0.f;

	//	ecs::SpriteComponent sprite;
	//	//sprite.Sprite = new );

	//	uint32_t entity3 = ecs::ECSManager::createEntity();
	//	ecs::ECSManager::addComponent<TransformComponent>(entity3, glm::vec3(position.x, position.y, position.z), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f));
	//	ecs::ECSManager::addComponent<SpriteComponent>(entity3, new Sprite(glm::vec3(position.x, position.y, position.z), glm::vec2(40, 40), TextureManager::get("test00")));
	//}

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Name = name;

	EditorManager::init();
}

void SetupView::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);
    //SceneManager::change("SelectProject");
    
    if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_F1))
        ecs::ECSManager::save();
	if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_F2))
		ecs::ECSManager::load();
}

void SetupView::Render()
{
	m_Shader->Enable();
	m_Camera->Render(m_Shader);
	//m_Layer->render();
}