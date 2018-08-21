#pragma once

#include <glm\glm.hpp>

struct TransformComponent
{
	TransformComponent() { };

	TransformComponent(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& scale)
		: position(position), rotation(rotation), scale(scale)
	{
	};

	glm::vec3 position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f }, scale{ 1.0f, 1.0f, 1.0f };
};

template<typename Archive>
void serialize(Archive &archive, TransformComponent &transform) {
    archive(transform.position.x, transform.position.y, transform.position.z);
    archive(transform.rotation.x, transform.rotation.y, transform.rotation.z);
    archive(transform.scale.x, transform.scale.y, transform.scale.z);
}