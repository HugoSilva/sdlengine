#pragma once

namespace rse {
class IndexBuffer {
 public:
  IndexBuffer(unsigned short* data, int count);
  IndexBuffer(unsigned int* data, int count);
  ~IndexBuffer();
  void Bind() const;
  void Unbind() const;

  inline unsigned int GetCount() const { return m_Count; }

 private:
  unsigned int m_BufferID;
  unsigned int m_Count;
};
}  // namespace rse