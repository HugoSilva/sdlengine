#include "Scene01.h"


SpriteTest::SpriteTest(SDL_Window* win)
{
	using namespace graphics;
	using namespace audio;

	FontManager::add(new Font("Arial", "Resources/arial.ttf", 24));

	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");

	//m_Layer = new Layer(new SDLRenderer(win), m_Shader);
	m_Layer = new Layer(new OpenGLRenderer(win), m_Shader);

	TextureManager::add(new Texture("test00", "Resources/tex3.png"));

	m_Sprite = new Sprite(glm::vec3(40, 40, 0), glm::vec2(40, 40), TextureManager::get("test00"));

	m_Sprite = new Sprite(glm::vec3(40, 40, 0), glm::vec2(40, 40), 0xaa0011ff);
	m_Layer->add(m_Sprite);

	m_Fps = new Label("FPS test", glm::vec3(20, 670, 0), FontManager::get("Arial"), 0xffffffff);
	m_Layer->add(m_Fps);

	Imgui* testImgui = new Imgui();
	m_Layer->add(testImgui);

	SoundManager::add(new Sound("eff", "Resources/effect.wav"));
	SoundManager::add(new Music("bgm", "Resources/background.ogg"));

	SoundManager::getMusic("bgm")->play();
	SoundManager::getSound("eff")->play();
	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));

	PhysicsManager::add(new Rigidbody(glm::vec2(0.0f, -10.0f), glm::vec2(50.0f, 10.0f)));
	PhysicsManager::add(new Rigidbody(glm::vec2(0.0f, 400.0f), glm::vec2(1.0f, 1.0f), true, true));
}

SpriteTest::~SpriteTest()
{
}

void SpriteTest::Update(float deltaTime)
{
	m_Camera->Update(deltaTime);
	PhysicsManager::Update(deltaTime, *m_Sprite);

	float speed = 0.5f;
	if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_UP))
		m_Sprite->position.y += speed;
	else if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_DOWN))
		m_Sprite->position.y -= speed;
	if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_LEFT))
		m_Sprite->position.x -= speed;
	else if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_RIGHT))
		m_Sprite->position.x += speed;

}

void SpriteTest::Render()
{
	m_Camera->Render(*m_Shader);
	m_Layer->render();
}