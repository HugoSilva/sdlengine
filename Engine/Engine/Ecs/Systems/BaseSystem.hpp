#pragma once

class BaseSystem
{
public:
	BaseSystem() {}
	virtual void update(const float alpha) = 0;
};