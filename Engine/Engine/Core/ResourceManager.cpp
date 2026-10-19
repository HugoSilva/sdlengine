#include "ResourceManager.hpp"

#include "Resource.hpp"

std::vector<Resource*> ResourceManager::s_Resources;

void ResourceManager::Init() {}

void ResourceManager::Add(Resource* resource) {
  for (std::vector<Resource*>::iterator it = s_Resources.begin(); it != s_Resources.end(); ++it) {
    Resource* res = *it;
    if (res->GetFileName().compare(resource->GetFileName()) == 0) return;
  }

  s_Resources.push_back(resource);
}

Resource* ResourceManager::Get(const std::string& name) {
  for (std::vector<Resource*>::iterator it = s_Resources.begin(); it != s_Resources.end(); ++it) {
    Resource* res = *it;
    if (res->GetFileName().compare(name) == 0) return res;
  }
  return nullptr;
}

void ResourceManager::Clean() { s_Resources.clear(); }