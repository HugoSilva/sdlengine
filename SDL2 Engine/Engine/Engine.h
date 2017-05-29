#ifndef ENGINE
#define ENGINE

#include <SDL.h>
#include <GL/glew.h>
#pragma comment(lib, "opengl32.lib")

#include "./Camera.h"
#include "Components/Shader.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>

class Engine
{
public:
	static int SCREEN_WIDTH;
	static int SCREEN_HEIGHT;
	static SDL_Window* GetWindow();

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	GLuint cameraViewMatUniform;
	GLuint cameraProjMatUniform;

	Engine();
	~Engine();

	bool Initialize(char* windowTitle);
	void SetCamera(Camera* camera);
	void Update(GLfloat deltaTime);
	void BeginRender();
	void EndRender();
	bool GetActive();
	void ProcessCamera(SDL_KeyboardEvent event, GLfloat deltaTime);
	void MouseCallback(SDL_MouseMotionEvent event);

private:
	static SDL_Window* window;
	static SDL_GLContext context;
	Camera* camera;
	bool active;
	float deltaTime = 0.0;
};

#endif
