#pragma once

#include "Components/Shader.h"

class Scene
{
public:

	virtual void Update(float deltaTime) = 0;

	virtual void Render() = 0;

	inline const std::string& getName() const { return m_Name; }

protected:
	std::string m_Name;
	Shader* m_Shader;
};