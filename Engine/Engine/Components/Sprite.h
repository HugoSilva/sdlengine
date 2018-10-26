#pragma once

#include <glm/glm.hpp>
#include "../Graphics/Renderable2D.h"
#include "../Graphics/Renderer2D.h"

#include <cereal/types/polymorphic.hpp>


class Sprite : public graphics::Renderable2D
{
public:
	Sprite();
	Sprite(glm::vec3 position, glm::vec2 size, unsigned int color);
	Sprite(glm::vec3 position, glm::vec2 size, Texture* texture);
	
	void submit(graphics::Renderer2D* renderer) {};
	glm::vec3& position;

	template<typename Archive>
	void serialize(Archive &archive)
	{
		archive(cereal::base_class<graphics::Renderable2D>(this), cereal::make_nvp("Position", position));
	}
};


CEREAL_REGISTER_TYPE(Sprite);

CEREAL_REGISTER_POLYMORPHIC_RELATION(graphics::Renderable2D, Sprite)