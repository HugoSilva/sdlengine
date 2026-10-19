#include "resource_manager.h"

#include "resource.h"

std::vector<Resource*> ResourceManager::s_resources;

void ResourceManager::Init() {}

void ResourceManager::Add(Resource* resource) {
  for (std::vector<Resource*>::iterator it = s_resources.begin(); it != s_resources.end(); ++it) {
    Resource* res = *it;
    if (res->GetFileName().compare(resource->GetFileName()) == 0) return;
  }

  s_resources.push_back(resource);
}

Resource* ResourceManager::Get(const std::string& name) {
  for (std::vector<Resource*>::iterator it = s_resources.begin(); it != s_resources.end(); ++it) {
    Resource* res = *it;
    if (res->GetFileName().compare(name) == 0) return res;
  }
  return nullptr;
}

void ResourceManager::Clean() { s_resources.clear(); }