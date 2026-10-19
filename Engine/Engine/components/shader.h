#pragma once

namespace rse {
class Shader {
 public:
  Shader(const char* vertex_path, const char* fragment_path);
  Shader(const char* name, const char* vertex_source, const char* fragment_source);
  ~Shader();

  void Enable() const;
  void Disable() const;

  unsigned int GetUniformLocation(const char* name);
  void SetUniform1i(const char* name, int value);
  void SetUniform1iv(const char* name, int* value, int count);
  void SetUniformMat4(const char* name, const float* matrix);

  static Shader* FromFile(const char* vertex_path, const char* fragment_path);
  static Shader* FromSource(const char* vertex_source, const char* fragment_source);
  static Shader* FromSource(const char* name, const char* vertex_source, const char* fragment_source);

  unsigned int GetShaderId() const { return m_shader_id; };

 private:
  unsigned int Load(const char* vertex_source, const char* fragment_source);

  unsigned int m_shader_id;
  const char* m_name;
  const char* m_vert_path;
  const char* m_frag_path;
  const char* m_vert_source;
  const char* m_frag_source;
};
}  // namespace rse