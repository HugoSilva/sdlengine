#pragma once

#include <vector>

class Resource; //forward declaration for Resource

class ResourceManager
{
public:
	static void init();
	static void add(Resource* sound);
	static Resource* get(const std::string& name);
	static void clean();

private:
	ResourceManager() { }
	static std::vector<Resource*> s_Resources;
};