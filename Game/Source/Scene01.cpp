#include "Scene01.h"

SpriteTest::SpriteTest(SDL_Window* win)
{
	using namespace graphics;
	using namespace audio;

	FontManager::add(new Font("Arial", "Resources/arial.ttf", 24));

#ifdef EMSCRIPTEN
	m_Shader = new Shader("Resources/ES3.vert", "Resources/ES3.frag");
#else
	m_Shader = new Shader("Resources/Default.vert", "Resources/Default.frag");
#endif // EMSCRIPTEN

	//m_Layer = new Layer(new SDLRenderer(win), m_Shader);
	m_Layer = new Layer(new OpenGLRenderer(win), m_Shader);

	TextureManager::add(new Texture("test00", "Resources/tex3.png"));
/*
	m_GroundSprite = new Sprite(glm::vec3(40, 40, 0), glm::vec2(40, 40), TextureManager::get("test00"));
	m_PlayerSprite = new Sprite(glm::vec3(40, 40, 0), glm::vec2(40, 40), 0xaa0011ff);

	m_Layer->add(m_GroundSprite);
	m_Layer->add(m_PlayerSprite);

	m_Fps = new Label("FPS test", glm::vec3(20, 670, 0), FontManager::get("Arial"), 0xffffffff);
	m_Layer->add(m_Fps);

	SoundManager::add(new Sound("eff", "./Resources/effect.wav"));
	SoundManager::add(new Music("bgm", "Resources/background.ogg"));

	SoundManager::getMusic("bgm")->play();
	SoundManager::getSound("eff")->play();
	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));

	PhysicsManager::add(new Rigidbody(m_GroundSprite, glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f)));
	PhysicsManager::add(new Rigidbody(m_PlayerSprite, glm::vec2(0.0f, 400.0f), glm::vec2(20.0f, 20.0f), true, true));
*/

	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));

	//TODO track down the memory leak with the new ECS 

	//New Entities code
	for (int i = 0; i < 10000; i++)
	{
		ecs::PositionComponent position;
		position.x = rand() % (1200 - 40 + 1) + 10;
		position.y = rand() % (720 - 40 + 1) + 10;
		position.z = 0.f;

		ecs::SpriteComponent sprite;

		sprite.Sprite = new Sprite(glm::vec3(position.x, position.y, position.z), glm::vec2(40, 40), TextureManager::get("test00"));

// 		int r = rand() % 256;
// 		int g = rand() % 256;
// 		int b = rand() % 256;
// 
// 		sprite.Color = 0xff << 24 | b << 16 | g << 8 | r;

		ecs::ECSManager::addEntity(position, sprite);
	}
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
	else if (IO::InputManager::IsKeyPressed(SDL_SCANCODE_F1))
		ecs::ECSManager::save();

}

void SpriteTest::Render()
{
	//TODO needs to be removed only has shader code for now
	m_Shader->Enable();
	//TODO create a camera system to replace old camera
	m_Camera->Render(m_Shader);
	//m_Layer->render();
}