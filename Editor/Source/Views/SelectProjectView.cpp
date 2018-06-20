#include "SelectProjectView.h"

#include "../Components/ImguiDialog.h"
#include "../Components/ImguiCreate.h"

SelectProjectView::SelectProjectView(SDL_Window* win)
{
	using namespace graphics;

	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");
	m_Layer = new Layer(new OpenGLRenderer(win), m_Shader);

	ImguiDialog* dialog = new ImguiDialog();
	m_Layer->add(dialog);

	ImguiCreate* create = new ImguiCreate();
	m_Layer->add(create);

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
}

void SelectProjectView::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);
}

void SelectProjectView::Render()
{
	m_Camera->Render(*m_Shader);
	m_Layer->render();
}