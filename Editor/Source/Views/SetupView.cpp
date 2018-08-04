#include "SetupView.h"

#include "../Components/ImguiMenuBar.h"
#include "../Components/ImguiDialog.h"
#include "../Components/ImguiCreate.h"

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

	ecs::DebugComponent mainbar;
	mainbar.element = new ImguiMenuBar();
	ecs::ECSManager::addEntity(mainbar);

	ecs::DebugComponent viewbar;
	viewbar.element = new ImguiDialog();
	ecs::ECSManager::addEntity(viewbar);

	ecs::DebugComponent componentbar;
	componentbar.element = new ImguiCreate();
	ecs::ECSManager::addEntity(componentbar);

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Name = name;
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