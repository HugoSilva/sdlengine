#pragma once

#include <Box2D.h>
#include "Updatable.h"
#include "..\Components\Sprite.h"

class Rigidbody : public physics::Updatable
{
public:
	Rigidbody();
	void Update(float deltaTime, Sprite sprite) override;

private:
	b2Body* m_body;
};