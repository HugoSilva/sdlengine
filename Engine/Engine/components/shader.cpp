#include "shader.h"

#include <string>
#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

#include <utils/file.h>

namespace rse {
Shader::Shader(const char* vertex_path, const char* fragment_path)
    : m_vert_path(vertex_path), m_frag_path(fragment_path) {
  std::string vert_string = File::Read(m_vert_path);
  std::string frag_string = File::Read(m_frag_path);

  m_shader_id = Load(vert_string.c_str(), frag_string.c_str());
}

Shader::Shader(const char* name, const char* vertex_source, const char* fragment_source)
    : m_name(name), m_vert_source(vertex_source), m_frag_source(fragment_source) {
  m_shader_id = Load(m_vert_source, m_frag_source);
}

Shader::~Shader() { glDeleteProgram(m_shader_id); }

void Shader::Enable() const { glUseProgram(m_shader_id); }

void Shader::Disable() const { glUseProgram(0); }

unsigned int Shader::GetUniformLocation(const char* name) {
  int result = glGetUniformLocation(m_shader_id, name);
  if (result == -1) {
    Logger::Error("Could not find uniform " + (std::string)name + " in shader!");
  }
  return result;
}

void Shader::SetUniform1i(const char* name, int value) { glUniform1i(GetUniformLocation(name), value); }

void Shader::SetUniform1iv(const char* name, int* value, int count) {
  glUniform1iv(GetUniformLocation(name), count, value);
}

void Shader::SetUniformMat4(const char* name, const float* matrix) {
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, matrix);
}

Shader* Shader::FromFile(const char* vertex_path, const char* fragment_path) {
  return new Shader(vertex_path, fragment_path);
}

Shader* Shader::FromSource(const char* vertex_source, const char* fragment_source) {
  return new Shader(vertex_source, fragment_source);
}

Shader* Shader::FromSource(const char* name, const char* vertex_source, const char* fragment_source) {
  return new Shader(name, vertex_source, fragment_source);
}

unsigned int Shader::Load(const char* vertex_source, const char* fragment_source) {
  unsigned int program = glCreateProgram();
  unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
  unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
  int success;
  char info_log[512];

  glShaderSource(vertex, 1, &vertex_source, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, info_log);
    Logger::Error("Shader error vertex compilation failed");
    std::string log(info_log);
    Logger::Warning(log);
  }

  glShaderSource(fragment, 1, &fragment_source, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, info_log);
    Logger::Error("Shader error fragment compilation failed");
    std::string log(info_log);
    Logger::Warning(log);
  }

  glAttachShader(program, vertex);
  glAttachShader(program, fragment);

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, info_log);
    Logger::Error("Shader error linking failed");
    std::string log(info_log);
    Logger::Warning(log);
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return program;
}
}  // namespace rse