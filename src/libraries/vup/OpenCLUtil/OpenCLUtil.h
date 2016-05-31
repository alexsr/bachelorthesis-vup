// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_OPENCLUTIL_H
#define VUP_OPENCLUTIL_H

#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/Util/FileReader.h"
#include "CL/cl.hpp"
#include "CL/cl_gl.h"
#include <vector>
#include <map>
#include <iostream>

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/OpenGL.h>
#ifdef UNIX
#include <GL/glx.h>
#endif
#endif

#if defined (__APPLE__) || defined(MACOSX)
#define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

namespace vup {

class OpenCLBasis
{
public:
  OpenCLBasis(int platformID, cl_device_type deviceType, int deviceID);
  ~OpenCLBasis();
  void setDefaultPlatform(int id);
  cl::Platform getPlatform() { return m_defaultPlatform; }
  void setDefaultDevice(int id);
  cl::Device getDevice() { return m_defaultDevice; }
  cl::Context getContext() { return m_context; }

private:
  void initPlatform(int id);
  void initContext(cl_device_type t);
  void initDevice(int id);

  cl::Platform m_defaultPlatform;
  std::vector<cl::Platform> m_platforms;
  cl::Device m_defaultDevice;
  std::vector<cl::Device> m_devices;
  cl::Context m_context;
};

class KernelRunner
{
public:
  KernelRunner(cl::Context context, cl::Device device, const char* programPath);
  ~KernelRunner();
  cl::CommandQueue getQueue() { return m_queue; }
  cl::Program getProgram() { return m_program; }
  void add(std::string name);
  cl::Kernel get(std::string name);
  template <class T> void setArg(std::string name, int index, T data);
  void writeBuffer(cl::Buffer b, cl_bool blocking, int offset, int size, const void * ptr);
  void runRangeKernel(std::string name, cl::NDRange offset, cl::NDRange global, cl::NDRange local);
  void acquireGL(std::vector<cl::Memory> * mem);
  void releaseGL(std::vector<cl::Memory> * mem);

  void finish() { m_queue.finish(); }
  void flush() { m_queue.flush(); }
private:
  bool doesKernelExist(std::string name);
  cl::CommandQueue m_queue;
  cl::Program m_program;
  std::map<std::string, cl::Kernel> m_kernels;
};
template<class T>
void KernelRunner::setArg(std::string name, int index, T data)
{
  m_kernels[name].setArg(index, data);
}
}

#endif
