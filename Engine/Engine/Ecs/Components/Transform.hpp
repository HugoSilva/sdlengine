#pragma once

#include <glm\glm.hpp>

struct TransformComponent
{
	TransformComponent() { };

	TransformComponent(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& scale)
		: position(position), rotation(rotation), scale(scale)
	{
	};

	glm::vec3 position, rotation, scale;
};