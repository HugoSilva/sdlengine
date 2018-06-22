#include "SetupView.h"

SetupView::SetupView(SDL_Window* win)
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

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
}

void SetupView::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);

	SceneManager::get("");
}

void SetupView::Render()
{
	m_Camera->Render(*m_Shader);
	m_Layer->render();
}