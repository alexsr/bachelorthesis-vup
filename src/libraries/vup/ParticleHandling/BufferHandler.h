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
  void addGL(std::string name, cl_mem_flags flags, GLuint vboHandle);
  cl::Buffer get(std::string name);
  cl::BufferGL getGL(std::string name);
  std::vector<cl::Memory> getGLBuffers() { return m_glBuffersVector; }
  cl::Context getDefaultContext() { return m_defaultContext; }
  
private:
  bool doesBufferExist(std::string name);
  bool doesGLBufferExist(std::string name);
  cl::Context m_defaultContext;
  std::map<std::string, cl::Buffer> m_buffers;
  std::map<std::string, cl::BufferGL> m_glBuffers;
  std::vector<cl::Memory> m_glBuffersVector;
};


}


#endif
