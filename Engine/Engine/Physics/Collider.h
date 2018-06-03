#pragma once

#include <Box2D.h>
#include "Updatable.h"
#include "..\Components\Sprite.h"

class Collider : public physics::Updatable
{
public:
	Collider();
	void Update() override;

private:
	b2Body* m_body;
};