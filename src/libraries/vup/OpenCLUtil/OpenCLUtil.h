// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_OPENCLUTIL_H
#define VUP_OPENCLUTIL_H

#include "vup/defs.h"
#include "vup/particle.h"
#include "CL/cl.hpp"
#include "CL/cl_gl.h"
#include <vector>
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

class TBD
{
public:
  TBD(int platformID, cl_device_type deviceType, int deviceID);
  ~TBD();
  void setDefaultPlatform(int id);
  cl::Platform getPlatform() { return m_defaultPlatform; }
  void setDefaultDevice(int id);
  cl::Device getDevice() { return m_defaultDevice; }
  cl::Context getContext() { return m_context; }
  cl::CommandQueue getQueue() { return m_queue; }

private:
  void initPlatform(int id);
  void initContext(cl_device_type t);
  void initDevice(int id);

  cl::Platform m_defaultPlatform;
  std::vector<cl::Platform> m_platforms;
  cl::Device m_defaultDevice;
  std::vector<cl::Device> m_devices;
  cl::Context m_context;
  cl::CommandQueue m_queue;
};

}

#endif
