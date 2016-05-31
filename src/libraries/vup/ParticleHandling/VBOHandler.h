// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_VBOHANDLER_H
#define VUP_VBOHANDLER_H

#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/Rendering/VBO.h"
#include <vector>
#include <map>
#include <iostream>

namespace vup {

class VBOHandler
{
public:
  VBOHandler();
  ~VBOHandler();
  template <typename T> void createVBO(std::string name, int loc, int size, int format, bool isInterop = false, GLint drawType = GL_STATIC_DRAW);
  template <typename T> void createVBOData(std::string name, int loc, int size, int format, std::vector<T> data, bool isInterop = false, GLint drawType = GL_STATIC_DRAW);
  template <typename T> void updateVBO(std::string name, std::vector<T> data);
  template <typename T> void updateSubVBO(std::string name, std::vector<T> data, int range);
  std::map<std::string, vup::VBO> getVBOs() { return m_vbos; }
  std::map<std::string, vup::VBO> getInteropVBOs() { return m_interopVBOs; }
  vup::VBO getVBO(std::string name);
  vup::VBO getInteropVBO(std::string name);
  GLuint getVBOHandle(std::string name);
  GLuint getInteropVBOHandle(std::string name);

private:
  std::map<std::string, vup::VBO> m_vbos;
  std::map<std::string, vup::VBO> m_interopVBOs;
};

template<typename T>
void vup::VBOHandler::createVBO(std::string name, int loc, int size, int format, bool isInterop, GLint drawType)
{
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(T) * size, NULL, drawType);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos[name] = vup::VBO(vbo, loc, format);
  if (isInterop) {
    m_interopVBOs[name] = vup::VBO(vbo, loc, format);
  }
}

template<typename T>
void vup::VBOHandler::createVBOData(std::string name, int loc, int size, int format, std::vector<T> data, bool isInterop, GLint drawType)
{
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(T) * size, &data[0], drawType);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos[name] = vup::VBO(vbo, loc, format);
  if (isInterop) {
    m_interopVBOs[name] = vup::VBO(vbo, loc, format);
  }
}

template<typename T>
void vup::VBOHandler::updateVBO(std::string name, std::vector<T> data)
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
void vup::VBOHandler::updateSubVBO(std::string name, std::vector<T> data, int range)
{
}


}


#endif
