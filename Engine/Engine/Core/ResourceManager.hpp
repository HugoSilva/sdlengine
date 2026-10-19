#pragma once

#include <string>
#include <vector>

class Resource;  // forward declaration for Resource

class ResourceManager {
 public:
  static void Init();
  static void Add(Resource* sound);
  static Resource* Get(const std::string& name);
  static void Clean();

 private:
  ResourceManager() {}
  static std::vector<Resource*> s_Resources;
};