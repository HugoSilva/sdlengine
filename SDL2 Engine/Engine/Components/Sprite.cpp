#include "Sprite.h"

using namespace std;

Sprite::Sprite()
{
	pos = glm::vec3(0);
	rot = 0;
	scale = glm::vec3(1);
	size = glm::vec3(0);
	texture = Texture();
}

Sprite::Sprite(string imagePath)
{
	string directory = imagePath.substr(0, imagePath.find_last_of('/'));
	texture = Texture(imagePath.c_str(), directory, "");
	pos = glm::vec3(0);
	scale = glm::vec3(1);
	size = glm::vec3((float)texture.GetWidth(), (float)texture.GetHeight(), 1);
	rot = 0;
}

Sprite::Sprite(string imagePath, glm::vec3 v)
{
	string directory = imagePath.substr(0, imagePath.find_last_of('/'));
	texture = Texture(imagePath.c_str(), directory, "");
	pos = v;
	scale = glm::vec3(1);
	size = glm::vec3((float)texture.GetWidth(), (float)texture.GetHeight(), 1);
	rot = 0;
}

void Sprite::Update()
{

}

void Sprite::Render()
{
}

void Sprite::MoveTo(glm::vec3 v)
{
	pos = v;
}

void Sprite::RotateTo(float x)
{
	rot = x;
}

void Sprite::SetScale(float x)
{
	scale = glm::vec3(x);
}

void Sprite::SetScale(glm::vec3 v)
{
	scale = v;
}

void Sprite::FlipHorizontal()
{
	scale.x = -scale.x;
}

void Sprite::FlipVertical()
{
	scale.y = -scale.y;
}

glm::vec3* Sprite::GetPos()
{
	return &pos;
}

float* Sprite::GetRot()
{
	return &rot;
}

glm::vec3* Sprite::GetScale()
{
	return &scale;
}

glm::vec3* Sprite::GetSize()
{
	return &size;
}