#pragma once

#include <string>

class Resource {
 public:
  Resource(const std::string& name, const std::string& filename);
  ~Resource();

  inline const std::string& GetName() const { return m_name; }
  inline const std::string& GetFileName() const { return m_filename; }

 private:
  std::string m_name;
  std::string m_filename;
};