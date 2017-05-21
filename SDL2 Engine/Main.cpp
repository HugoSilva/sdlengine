#include "Engine/Engine.h"
//#include "Engine/Components/Sprite.h"
//#include "Engine/Components/Model.h"
//#include "Engine/Components/Shader.h"

#include <iostream>
using namespace std;

void drawCube() {
	glBegin(GL_QUADS);

	glColor3f(0.0f, 1.0f, 0.0f);     // Green
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	// Bottom face (y = -1.0f)
	glColor3f(1.0f, 0.5f, 0.0f);     // Orange
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	// Front face  (z = 1.0f)
	glColor3f(1.0f, 0.0f, 0.0f);     // Red
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	// Back face (z = -1.0f)
	glColor3f(1.0f, 1.0f, 0.0f);     // Yellow
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// Left face (x = -1.0f)
	glColor3f(0.0f, 0.0f, 1.0f);     // Blue
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	// Right face (x = 1.0f)
	glColor3f(1.0f, 0.0f, 1.0f);     // Magenta
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	glEnd();  // End of drawing color-cube
}

int main(int argc, char **argv)
{
	Engine engine;
	bool validInit = engine.Initialize("Lego Demo!");

	if (validInit) {

		//Sprite testSprite = Sprite("Assets/Textures/texture.png", Vector3(Engine::SCREEN_WIDTH / 2, Engine::SCREEN_HEIGHT / 2, 0));
		//Shader shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		//Model model("Assets/Models/stormtrooper.fbx");
		int i = 0;
		while (engine.GetActive()) {

			Uint32 deltaTime = SDL_GetTicks();

			engine.Update();
			engine.BeginRender();
			//testSprite.Render();
			//model.Draw(shader);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();                 // Reset the model-view matrix
			glRotatef(i++, 1.0f, 1.0f, 0.0f);
			drawCube();

			engine.EndRender();
		}
	}

	engine.~Engine();

	return EXIT_SUCCESS;
}