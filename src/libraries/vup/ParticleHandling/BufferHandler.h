// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BUFFERHANDLER_H
#define VUP_BUFFERHANDLER_H

#include "vup/defs.h"
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

// Manages OpenCL and OpenGL buffers and provides functions to create new buffers
// and possibly fill them with data already.
class BufferHandler
{
public:
  // Sets context of OpenCL buffers to defaultContext.
  BufferHandler(cl::Context defaultContext);
  ~BufferHandler();
 
  // Clears all buffer maps.
  void clear();
  cl::Context getDefaultContext() { return m_defaultContext; }

  // OpenCL buffer handling

  // Creates a buffer for data of type T with the specified size and mem flags.
  // The name acts as the key in the m_buffers map.
  // Throws an exception if the buffer creation fails.
  template <typename T> void createBuffer(std::string name, cl_mem_flags flags, int size);
  // Adds an existing buffer to the m_buffers map.
  // Throws an exception if the buffer already exists.
  void addBuffer(std::string name, cl::Buffer buffer);
  // Creates a cl::BufferGL from an OpenGL buffer that was previously created or added as an interop-VBO.
  // Adds the buffer to m_glBuffers.
  // Throws an exception if the buffer creation fails.
  void createBufferGL(std::string name, cl_mem_flags flags, std::string vbo);
  // Adds an existing bufferGL to the m_glBuffers map.
  // Throws an exception if the buffer already exists.
  void addBufferGL(std::string name, cl::BufferGL buffer);
  cl::Buffer getBuffer(std::string name);
  cl::BufferGL getBufferGL(std::string name);
  // Returns the OpenCL buffers of interop buffers as a vector of memory objects.
  // This is necessary because the acquire and release functions of the queue require memory objects.
  std::vector<cl::Memory> * getGLBuffers() { return m_glBuffersVector; }

  // OpenGL buffer handling

  // Creates an OpenGL VBO for an element count equal to size of a given data type T.
  // loc specifies the layout location of the VBO in the shader and format specifies
  // the how many data elements an element on the shader is comprised of.
  // This information is used by the renderer when the vbo is associated with a vao.
  template <typename T> void createVBO(std::string name, int loc, int size, int format, bool isInterop = false, GLint drawType = GL_STATIC_DRAW);
  // Fills the VBO with data already. For further information refer to createVBO.
  template <typename T> void createVBOData(std::string name, int loc, int size, int format, std::vector<T> data, bool isInterop = false, GLint drawType = GL_STATIC_DRAW);
  // Update the whole VBO with the specified name with data of type T.
  template <typename T> void updateVBO(std::string name, std::vector<T> data);
  // Update the part of the VBO with the specified name starting at offset and ending at offset + length with data of type T.
  template <typename T> void updateSubVBO(std::string name, std::vector<T> data, int offset, int length);
  std::map<std::string, vup::VBO> getVBOs() { return m_vbos; }
  std::map<std::string, vup::VBO> getInteropVBOs() { return m_interopVBOs; }
  vup::VBO getVBO(std::string name);
  vup::VBO getInteropVBO(std::string name);
  GLuint getVBOHandle(std::string name);
  GLuint getInteropVBOHandle(std::string name);
  
private:
  bool doesBufferExist(std::string name);
  bool doesBufferGLExist(std::string name);

  cl::Context m_defaultContext;
  std::map<std::string, cl::Buffer> m_buffers;
  std::map<std::string, cl::BufferGL> m_glBuffers;
  std::vector<cl::Memory> * m_glBuffersVector;
  std::map<std::string, vup::VBO> m_vbos;
  std::map<std::string, vup::VBO> m_interopVBOs;
};

template<typename T>
void BufferHandler::createBuffer(std::string name, cl_mem_flags flags, int size)
{
  if (doesBufferExist(name)) {
    // -1 is used here because it is not an OpenCL error code and therefore clearly distinguishable.
    throw new BufferCreationException(name, -1);
  }
  cl_int clError;
  m_buffers[name] = cl::Buffer(m_defaultContext, flags, size * sizeof(T), nullptr, &clError);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException(name, clError);
  }
}

template<typename T>
void BufferHandler::createVBO(std::string name, int loc, int size, int format, bool isInterop, GLint drawType)
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
void BufferHandler::createVBOData(std::string name, int loc, int size, int format, std::vector<T> data, bool isInterop, GLint drawType)
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
void BufferHandler::updateVBO(std::string name, std::vector<T> data)
{
  glBindBuffer(GL_ARRAY_BUFFER, getVBOHandle(name));
  // Unfortunately casting to data is expected by OpenGL here.
  // Using glBufferData did not work for unknown reasons.
  // vertexArray points to the data in the VBO and therefore its data can be manipulated directly.
  T * vertexArray = (T *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (int i = 0; i < data->size(); i++) {
    vertexArray[i] = data->at(i);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename T>
void BufferHandler::updateSubVBO(std::string name, std::vector<T> data, int offset, int length)
{
  glBindBuffer(GL_ARRAY_BUFFER, getVBOHandle(name));
  glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), length * sizeof(T), &data[0]);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}


#endif
