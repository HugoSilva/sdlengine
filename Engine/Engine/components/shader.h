#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace rse {
class Shader {
 public:
  Shader(const char* vertex_path, const char* fragment_path);
  Shader(const char* name, const char* vertex_source, const char* fragment_source);
  ~Shader();

  void Enable() const;
  void Disable() const;

  // Where a uniform sits in this program, or -1 when the program has no uniform
  // by that name. Resolved once: a program's uniform layout is fixed when it
  // links, so every later call is a lookup in a table rather than a driver call.
  int GetUniformLocation(const char* name);

  // True when the program declares this uniform, without logging a miss. A
  // material shader may implement only the texture slots it samples, so the
  // renderer asks before uploading each unit instead of logging an error for a
  // unit the shader never reads.
  bool HasUniform(const char* name);

  void SetUniform1i(const char* name, int value);
  void SetUniform1iv(const char* name, int* value, int count);
  void SetUniformMat4(const char* name, const float* matrix);

  static Shader* FromFile(const char* vertex_path, const char* fragment_path);
  static Shader* FromSource(const char* vertex_source, const char* fragment_source);
  static Shader* FromSource(const char* name, const char* vertex_source, const char* fragment_source);

  unsigned int GetShaderId() const { return m_shader_id; };

 private:
  unsigned int Load(const char* vertex_source, const char* fragment_source);

  // The cached lookup both public queries share. It never logs.
  int FindUniformLocation(const char* name);

  unsigned int m_shader_id;
  const char* m_name;
  const char* m_vert_path;
  const char* m_frag_path;
  const char* m_vert_source;
  const char* m_frag_source;

  // Uniform name -> location, misses included (-1). A miss is worth caching:
  // the answer cannot change. GetUniformLocation reports a missing uniform once
  // per name (m_reported_misses), so a frame that re-uploads an optional uniform
  // does not flood the log.
  std::unordered_map<std::string, int> m_uniform_locations;
  std::unordered_set<std::string> m_reported_misses;
};
}  // namespace rse
