#include "Engine.h"
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

using namespace std;

int Engine::SCREEN_WIDTH = 1280;
int Engine::SCREEN_HEIGHT = 720;
SDL_Window* Engine::window = NULL;
SDL_GLContext Engine::context = NULL;
SDL_Renderer* Engine::renderer = NULL;
SDL_Surface* Engine::surface = NULL;
//Camera Engine::camera = NULL;

Engine::Engine()
{

}

Engine::~Engine()
{
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool Engine::Initialize(char* windowTitle)
{

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	//renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); 
	surface = SDL_GetWindowSurface(window);

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


	this->camera = Camera::Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2.0, 2.0, -2.0, 2.0, -20.0, 20.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//// TODO Should I enable back face culling with counter-clockwise winding for front faces
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	SDL_GL_SetSwapInterval(1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	return true;
}

void Engine::ProcessCamera(SDL_KeyboardEvent event)
{

	SDL_Scancode key = event.keysym.scancode;
	if (key == SDL_SCANCODE_UP)
	{
		this->camera.ProcessKeyboard(FORWARD, 1.0f);
	}

	if (key == SDL_SCANCODE_DOWN)
	{
		this->camera.ProcessKeyboard(BACKWARD, 1.0f);
	}
}

void Engine::Update()
{
	SDL_Event windowEvent;

	if (SDL_PollEvent(&windowEvent)) {
		switch (windowEvent.type) {
		case SDL_QUIT:
			active = false;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			Keyboard::KeyCallback(windowEvent.key);
			ProcessCamera(windowEvent.key);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			Mouse::MouseButtonCallback(windowEvent.button);
		case SDL_MOUSEMOTION:
			Mouse::MousePosCallback(windowEvent.motion);
			break;
		}
	}
	//cameraProjection = glm::perspective(fieldOfView, (float)displayWidth / (float)displayHeight, zNear, zFar);
	//cameraView = glm::inverse(glm::translate(cameraPosition) * glm::mat4_cast(glm::quat(cameraRotation)) * glm::scale(glm::vec3(1.0f)));
	//modelMatrix = glm::translate(modelPosition) * glm::mat4_cast(glm::quat(modelRotation)) * glm::scale(modelScale);


	glm::mat4 view = camera.GetViewMatrix();
}

void Engine::BeginRender()
{
	SDL_GL_MakeCurrent(window, context);
	//SDL_RenderClear(renderer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::EndRender()
{
	SDL_GL_SwapWindow(window);
	//SDL_RenderPresent(renderer);
}

SDL_Window* Engine::GetWindow()
{
	return window;
}

SDL_Renderer* Engine::GetRenderer()
{
	return renderer;
}

SDL_Surface* Engine::GetSurface()
{
	return surface;
}

bool Engine::GetActive()
{
	return this->active;
}