#pragma once

#include "Components/Shader.h"
#include "Camera.h"

class Scene
{
public:

	virtual void Update(float deltaTime) = 0;

	virtual void Render() = 0;

private:
	Shader* shader;
	Camera* camera;
};