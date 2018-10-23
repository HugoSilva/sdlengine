#pragma once

#include <glm\glm.hpp>
#include <cereal/access.hpp> // For LoadAndConstruct
#include "../../Components/Sprite.h"

struct SpriteComponent
{
	SpriteComponent() { };

	SpriteComponent(Sprite* sprite) : m_Sprite(sprite)
	{
	};
	
	std::unique_ptr<Sprite> m_Sprite;
};

template<typename Archive>
void serialize(Archive &archive, SpriteComponent &sprite)
{
	archive(cereal::make_nvp("Sprite", sprite.m_Sprite));
}

namespace cereal
{
	template <class Archive>
	static void load_and_construct(Archive& _archive, cereal::construct<SpriteComponent>& _construct)
	{
		std::unique_ptr<Sprite> strs;
		_archive(strs);
		_construct(*strs);
	}
}