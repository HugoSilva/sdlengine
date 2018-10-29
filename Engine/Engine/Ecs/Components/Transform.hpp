#pragma once

#include <glm\glm.hpp>

struct TransformComponent
{
	TransformComponent() { };

	TransformComponent(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		: position(position), rotation(rotation), scale(scale)
	{
	};

	glm::vec3 position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f }, scale{ 1.0f, 1.0f, 1.0f };

	template<typename Archive>
	void serialize(Archive &archive) {
		archive(cereal::make_nvp("Position", position), cereal::make_nvp("Rotation", rotation), cereal::make_nvp("Scale", scale));
	}
};

namespace glm
{
    template<typename Archive> void serialize(Archive& archive, glm::vec2& v2)
    {
        archive(cereal::make_nvp("x", v2.x), cereal::make_nvp("y", v2.y));
    }

	template<typename Archive> void serialize(Archive& archive, glm::vec3& v3)
	{
		archive(cereal::make_nvp("x", v3.x), cereal::make_nvp("y", v3.y), cereal::make_nvp("z", v3.z));
	}
}

