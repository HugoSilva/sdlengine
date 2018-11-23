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

	using namespace rse;

	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");
	m_Layer = new Layer(new OpenGLRenderer(win, m_Shader), m_Shader);

	//TODO I need to add a function to load resources in the project
	TextureManager::add(new Texture("test00", "Resources/tex3.png"));
	SoundManager::add(new Sound("eff", "Resources/effect.wav"));

	//ImguiMenuBar* menubar = new ImguiMenuBar();
	//m_Layer->add(menubar);
	m_render = new RenderSystem();
	rse::ECSManager::addSystem(m_render);

	uint32_t entity = rse::ECSManager::createEditorEntity();
	rse::ECSManager::addEditorComponent<EditorComponent>(entity, new MenuBarComponent());

	uint32_t entity1 = rse::ECSManager::createEditorEntity();
	rse::ECSManager::addEditorComponent<EditorComponent>(entity1, new EntityViewerComponent());

	uint32_t entity2 = rse::ECSManager::createEditorEntity();
	rse::ECSManager::addEditorComponent<EditorComponent>(entity2, new PropertyComponent());

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

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Name = name;

	EditorManager::init();
}

void SetupView::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);
    //SceneManager::change("SelectProject");
	
	if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F1))
		rse::ECSManager::save();
	if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F2))
		rse::ECSManager::load();
}

void SetupView::Render()
{
	m_Shader->enable();
	m_Camera->Render(m_Shader);
	//m_Layer->render();
}