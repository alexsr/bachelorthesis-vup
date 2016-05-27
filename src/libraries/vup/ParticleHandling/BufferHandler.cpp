#include "BufferHandler.h"

vup::BufferHandler::BufferHandler()
{
  m_vbos = std::map<std::string, vup::VBO>();
  m_interopVBOs = std::map<std::string, vup::VBO>();
}

vup::BufferHandler::~BufferHandler()
{
}

vup::VBO vup::BufferHandler::getVBO(std::string name)
{
  std::map<std::string, vup::VBO>::iterator it = m_vbos.find(name);
  if (it != m_vbos.end())
  {
    return it->second;
  }
  else
  {
    // TODO: throw Exception
    throw std::exception();
  }
}

vup::VBO vup::BufferHandler::getInteropVBO(std::string name)
{
  std::map<std::string, vup::VBO>::iterator it = m_interopVBOs.find(name);
  if (it != m_interopVBOs.end())
  {
    return it->second;
  }
  else
  {
    // TODO: throw Exception
    throw std::exception();
  }
}

GLuint vup::BufferHandler::getVBOHandle(std::string name)
{
  return getVBO(name).handle;
}

GLuint vup::BufferHandler::getInteropVBOHandle(std::string name)
{
  return getInteropVBO(name).handle;
}

template<typename T>
void vup::BufferHandler::createVBO(std::string name, int loc, int size, bool isInterop, GLint drawType)
{
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(T) * size, NULL, drawType);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos[name] = vup::VBO(vbo, loc, sizeof(T));
  if (isInterop) {
    m_interopVBOs[name] = vup::VBO(vbo, loc, sizeof(T));
  }
}

template<>
void vup::BufferHandler::createVBO<glm::vec4>(std::string name, int loc, int size, bool isInterop, GLint drawType)
{
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * size, NULL, drawType);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos[name] = vup::VBO(vbo, loc, 4);
  if (isInterop) {
    m_interopVBOs[name] = vup::VBO(vbo, loc, 4);
  }
}

template<typename T>
void vup::BufferHandler::createVBOData(std::string name, int loc, int size, std::vector<T> data, bool isInterop, GLint drawType)
{
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(T) * size, &data[0], drawType);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos[name] = vup::VBO(vbo, loc, T);
  if (isInterop) {
    m_interopVBOs[name] = vup::VBO(vbo, loc, T);
  }
}

template<>
void vup::BufferHandler::createVBOData<glm::vec4>(std::string name, int loc, int size, std::vector<glm::vec4> data, bool isInterop, GLint drawType)
{
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * size, &data[0], drawType);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos[name] = vup::VBO(vbo, loc, 4);
  if (isInterop) {
    m_interopVBOs[name] = vup::VBO(vbo, loc, 4);
  }
}

template<typename T>
void vup::BufferHandler::updateVBO(std::string name, std::vector<T> data)
{
  glBindBuffer(GL_ARRAY_BUFFER, getVBOHandle(name));
  T * vertexArray = (T *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (int i = 0; i < data->size(); i++) {
    vertexArray[i] = data->at(i);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename T>
void vup::BufferHandler::updateSubVBO(std::string name, std::vector<T> data, int range)
{
}
