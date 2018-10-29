#pragma once

#include "../../Graphics/Renderer2D.h"

class BaseSystem
{
public:
	BaseSystem() {}
	virtual void update(const float alpha) = 0;
};