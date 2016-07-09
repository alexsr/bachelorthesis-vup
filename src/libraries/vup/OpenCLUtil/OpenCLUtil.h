// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_OPENCLUTIL_H
#define VUP_OPENCLUTIL_H

#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/Util/FileReader.h"
#include "vup/Exceptions/KernelNotFoundException.h"
#include "vup/Exceptions/KernelCreationException.h"
#include "vup/Exceptions/RunKernelException.h"
#include "vup/Exceptions/CLProgramCompilationException.h"
#include "vup/Exceptions/AcquiringGLObjectsException.h"
#include "vup/Exceptions/ReleasingGLObjectsException.h"
#include "vup/Exceptions/BufferWritingException.h"
#include "vup/Exceptions/BufferNotFoundException.h"
#include "vup/ParticleHandling/TypeBuffer.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#ifdef UNIX
#include <GL/glx.h>
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
  cl::Platform platform() { return m_defaultPlatform; }
  cl::Device device() { return m_defaultDevice; }
  cl::Context context() { return m_context; }

private:
  cl::Platform m_defaultPlatform;
  std::vector<cl::Platform> m_platforms;
  cl::Device m_defaultDevice;
  std::vector<cl::Device> m_devices;
  cl::Context m_context;
};

class KernelHandler
{
public:
  KernelHandler(cl::Context context, cl::Device device, const char* path);
  KernelHandler(cl::Context context, cl::Device device, const char* path, std::vector<std::string> kernels);
  ~KernelHandler();
  void reloadProgram();
  void reloadProgram(const char* path);
  void reloadProgram(cl::Context context, cl::Device device, const char* path);
  void initKernels(std::vector<std::string> kernels);
  void initKernel(std::string kernel);
  cl::Kernel get(std::string name);
  template <class T> void setArg(std::vector<std::string> names, int index, T data);
  template <class T> void setArg(const char* name, int index, T data);
private:
  void buildProgram(cl::Context context, cl::Device device, const char* path);
  bool doesKernelExist(std::string name);
  cl::Context m_context;
  cl::Device m_device;
  cl::Program m_program;
  const char* m_path;
  std::map<std::string, cl::Kernel> m_kernels;
};
template<class T>
void KernelHandler::setArg(std::vector<std::string> names, int index, T data)
{
  for (std::string name : names) {
    m_kernels[name].setArg(index, data);
  }
}

template<class T>
void KernelHandler::setArg(const char* name, int index, T data)
{
  m_kernels[name].setArg(index, data);
}

class Queue
{
public:
  Queue(cl::Context context);
  ~Queue();
  cl::CommandQueue getQueue() { return m_queue; }
  void writeBuffer(cl::Buffer b, int size, const void * ptr);
  void writeBuffer(cl::Buffer b, cl_bool blocking, int offset, int size, const void * ptr);
  void runRangeKernel(cl::Kernel k, int global);
  void runRangeKernel(cl::Kernel k, int offset, int global, int local);
  void acquireGL(std::vector<cl::Memory> * mem);
  void releaseGL(std::vector<cl::Memory> * mem);

  void finish() { m_queue.finish(); }
  void flush() { m_queue.flush(); }

protected:
  cl::CommandQueue m_queue;
  cl::Context m_context;
};

class ParticleQueue : public vup::Queue
{
public:
  ParticleQueue(cl::Context context, int particleAmount);
  ~ParticleQueue();
  void runKernelOnType(cl::Kernel k, int type);
  void setTypeIndices(int type, cl_mem_flags flags, std::vector<int> indices, cl_bool blocking);
  std::vector<int> getIndices(int type);
  int getIndicesAmount(int type);
  cl::Buffer getIndexBuffer(int type);
  void addIndices(int type, std::vector<int> indices);
  void removeIndices(int type, std::vector<int> indices);
private:
  bool doesTypeExist(int type);
  std::map<int, vup::TypeBuffer> m_typeIndices;
  int m_particleAmount;
};

}

#endif
