#ifndef SPRITE
#define SPRITE

#include "Texture.h"
#include <glm/glm.hpp>
#include "../Engine.h"

#include <iostream>
#include <string>
using namespace std;

class Sprite
{
public:
	Sprite();
	Sprite(string imagePath);
	Sprite(string imagePath, glm::vec3 _pos);

	void Update();
	void Render();

	void MoveTo(glm::vec3 v);
	void RotateTo(float x);

	void SetScale(float x);
	void SetScale(glm::vec3 v);

	void FlipHorizontal();
	void FlipVertical();

	glm::vec3* GetPos();
	float* GetRot();
	glm::vec3* GetScale();
	glm::vec3* GetSize();

private:
	Texture texture;
	glm::vec3 pos;
	float rot;
	glm::vec3 scale;
	glm::vec3 size;
};

#endif