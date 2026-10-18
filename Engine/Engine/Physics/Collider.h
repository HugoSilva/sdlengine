#pragma once

#include <box2d/box2d.h>
#include "Updatable.h"
#include "../Components/Sprite.hpp"

class Collider : public physics::Updatable
{
public:
	Collider();
	void Update() override;

private:
	b2Body* m_body;
};