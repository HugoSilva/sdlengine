#include "WorkSpaceView.h"

#include "../Components/ImguiMenuBar.h"

WorkSpaceView::WorkSpaceView(std::string name, SDL_Window* win)
{
	using namespace graphics;

	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");
	m_Layer = new Layer(new OpenGLRenderer(win), m_Shader);

	ImguiMenuBar* menubar = new ImguiMenuBar();
	m_Layer->add(menubar);

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Name = name;
}

void WorkSpaceView::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);
}

void WorkSpaceView::Render()
{
	m_Camera->Render(m_Shader);
	//m_Layer->render();
}