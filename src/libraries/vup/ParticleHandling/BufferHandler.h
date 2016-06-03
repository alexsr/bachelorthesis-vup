// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BUFFERHANDLER_H
#define VUP_BUFFERHANDLER_H

#include "vup/defs.h"
#include "vup/particle.h"
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.hpp>
#include <CL/cl_gl.h>
#endif
#include "vup/ParticleHandling/VBO.h"
#include "vup/Exceptions/BufferCreationException.h"
#include "vup/Exceptions/BufferNotFoundException.h"
#include <vector>
#include <map>
#include <iostream>

namespace vup {

class BufferHandler
{
public:
  BufferHandler(cl::Context defaultContext);
  ~BufferHandler();
  void add(std::string name, cl_mem_flags flags, int size);
  void add(std::string name, cl::Buffer buffer);
  void addGL(std::string name, cl_mem_flags flags, std::string vbo );
  void addGL(std::string name, cl::BufferGL buffer);
  cl::Buffer get(std::string name);
  cl::BufferGL getGL(std::string name);
  std::vector<cl::Memory> getGLBuffers() { return m_glBuffersVector; }
  cl::Context getDefaultContext() { return m_defaultContext; }

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
  bool doesBufferExist(std::string name);
  bool doesGLBufferExist(std::string name);
  cl::Context m_defaultContext;
  std::map<std::string, cl::Buffer> m_buffers;
  std::map<std::string, cl::BufferGL> m_glBuffers;
  std::vector<cl::Memory> m_glBuffersVector;
  std::map<std::string, vup::VBO> m_vbos;
  std::map<std::string, vup::VBO> m_interopVBOs;
};

template<typename T>
void vup::BufferHandler::createVBO(std::string name, int loc, int size, int format, bool isInterop, GLint drawType)
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
void vup::BufferHandler::createVBOData(std::string name, int loc, int size, int format, std::vector<T> data, bool isInterop, GLint drawType)
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

}


#endif
