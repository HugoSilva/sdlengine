#include "Engine.h"
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

using namespace std;

int Engine::SCREEN_WIDTH = 1280;
int Engine::SCREEN_HEIGHT = 720;
SDL_Window* Engine::window = NULL;
SDL_GLContext Engine::context = NULL;
GLfloat lastX = Engine::SCREEN_WIDTH/2, lastY = Engine::SCREEN_HEIGHT/2;
bool firstMouse = true;

Engine::Engine()
{

}

Engine::~Engine()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool Engine::Initialize(char* windowTitle)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(0, &mode);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		cout << "Failed to create SDL Window: " << SDL_GetError() << endl;
		SDL_Quit();
		return false;
	}

	context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	GLenum result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "Failed to initialize GLEW: "<< glewGetErrorString(result) << endl;
		return false;
	}

	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetWindowGrab(window, SDL_TRUE);
	//TODO verify the SDL_GL_SetSwapInterval setting
	SDL_GL_SetSwapInterval(1);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	this->active = true;

	return true;
}

void Engine::SetCamera(Camera* camera)
{
	this->camera = camera;
}

void Engine::ProcessCamera(SDL_KeyboardEvent event, GLfloat deltaTime)
{

	SDL_Scancode key = event.keysym.scancode;
	if (key == SDL_SCANCODE_UP)
	{
		this->camera->ProcessKeyboard(FORWARD, deltaTime);
	}

	if (key == SDL_SCANCODE_DOWN)
	{
		this->camera->ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (key == SDL_SCANCODE_LEFT)
	{
		this->camera->ProcessKeyboard(LEFT, deltaTime);
	}

	if (key == SDL_SCANCODE_RIGHT)
	{
		this->camera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

void Engine::MouseCallback(SDL_MouseMotionEvent event)
{
	int width, height;
	SDL_GetWindowSize(SDL_GetWindowFromID(event.windowID), &width, &height);

	double xPos = event.x;
	double yPos = event.y;

	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	this->camera->ProcessMouseMovement(xOffset, yOffset);
}


void Engine::Update(GLfloat deltaTime)
{
	this->deltaTime = deltaTime;
	SDL_Event windowEvent;

	if (SDL_PollEvent(&windowEvent)) {
		switch (windowEvent.type) {
		case SDL_QUIT:
			active = false;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			Keyboard::KeyCallback(windowEvent.key);
			ProcessCamera(windowEvent.key, this->deltaTime);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			Mouse::MouseButtonCallback(windowEvent.button);
		case SDL_MOUSEMOTION:
			Mouse::MousePosCallback(windowEvent.motion);
			MouseCallback(windowEvent.motion);
			break;
		}
	}
}

void Engine::BeginRender()
{
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::EndRender()
{
	SDL_GL_SwapWindow(window);
}

SDL_Window* Engine::GetWindow()
{
	return window;
}

bool Engine::GetActive()
{
	return this->active;
}