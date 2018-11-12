#pragma once

#include <glm\glm.hpp>
#include "../../Components/Sprite.hpp"

struct SpriteComponent
{
	SpriteComponent() { };

	SpriteComponent(rse::Sprite* sprite) : m_Sprite(sprite)
	{
	};

	std::shared_ptr<rse::Sprite> m_Sprite;

	template<typename Archive>
	void serialize(Archive &archive)
	{
		archive(cereal::make_nvp("Sprite", m_Sprite));
	}
};
