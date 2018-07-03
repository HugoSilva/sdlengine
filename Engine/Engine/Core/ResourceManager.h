#pragma once

#include <vector>

#include "Resource.h"
#include "../Utils/Logger.h"

class ResourceManager
{
public:
	static void init();
	static void add(Resource* sound);
	static Resource* get(const std::string& name);
	static void clean();

private:
	ResourceManager() { }
	static std::vector<Resource*> m_Resources;
};