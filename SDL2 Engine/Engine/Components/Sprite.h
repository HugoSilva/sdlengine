#ifndef SPRITE
#define SPRITE

#include "Texture.h"
#include "../Math/Vector3.h"
#include "../Engine.h"

#include <iostream>
#include <string>
using namespace std;

class Sprite
{
public:
	Sprite();
	Sprite(string imagePath);
	Sprite(string imagePath, Vector3 _pos);

	void Update();
	void Render();

	void MoveTo(Vector3 v);
	void RotateTo(float x);

	void SetScale(float x);
	void SetScale(Vector3 v);

	void FlipHorizontal();
	void FlipVertical();

	Vector3* GetPos();
	float* GetRot();
	Vector3* GetScale();
	Vector3* GetSize();

private:
	Texture texture;
	Vector3 pos;
	float rot;
	Vector3 scale;
	Vector3 size;
};

#endif