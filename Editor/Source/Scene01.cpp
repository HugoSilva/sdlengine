#include "Scene01.h"

#include "ImguiDialog.h"

SpriteTest::SpriteTest(SDL_Window* win)
{
	/*
	Editor Work::
	----------------------------
	- Work flow
	-- Select which project to work
	-- Load Workspace
	-- Load last scene
	*/

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

	ImguiMenuBar* menubar = new ImguiMenuBar();
	m_Layer->add(menubar);

	ImguiDialog* dialog = new ImguiDialog();
	m_Layer->add(dialog);

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
}

SpriteTest::~SpriteTest()
{
}

void SpriteTest::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);

	float speed = 0.1f;
	if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_UP))
		m_PlayerSprite->position.y += speed;
	else if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_DOWN))
		m_PlayerSprite->position.y -= speed;
	if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_LEFT))
		m_PlayerSprite->position.x -= speed;
	else if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_RIGHT))
		m_PlayerSprite->position.x += speed;

}

void SpriteTest::Render()
{
	m_Camera->Render(*m_Shader);
	m_Layer->render();
}