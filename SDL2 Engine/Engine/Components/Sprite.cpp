#include "Sprite.h"
#include "../Engine.h"

Sprite::Sprite()
{
	pos = Vector3(0);
	rot = 0;
	scale = Vector3(1);
	size = Vector3(0);
	texture = Texture();
}

Sprite::Sprite(string imagePath)
{
	texture = Texture(imagePath);
	pos = Vector3(0);
	scale = Vector3(1);
	size = Vector3((float)texture.GetWidth(), (float)texture.GetHeight(), 1);
	rot = 0;
}

Sprite::Sprite(string imagePath, Vector3 v)
{
	texture = Texture(imagePath);
	pos = v;
	scale = Vector3(1);
	size = Vector3((float)texture.GetWidth(), (float)texture.GetHeight(), 1);
	rot = 0;
}

void Sprite::Update()
{

}

void Sprite::Render()
{
	SDL_BlitSurface(texture.GetSurface(), NULL, Engine::GetSurface(), NULL);
	SDL_UpdateWindowSurface(Engine::GetWindow());
	//SDL_RenderCopy(Engine::GetRenderer(), texture.GetTexture(), NULL, NULL);
}

void Sprite::MoveTo(Vector3 v)
{
	pos = v;
}

void Sprite::RotateTo(float x)
{
	rot = x;
}

void Sprite::SetScale(float x)
{
	scale = Vector3(x);
}

void Sprite::SetScale(Vector3 v)
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

Vector3* Sprite::GetPos()
{
	return &pos;
}

float* Sprite::GetRot()
{
	return &rot;
}

Vector3* Sprite::GetScale()
{
	return &scale;
}

Vector3* Sprite::GetSize()
{
	return &size;
}