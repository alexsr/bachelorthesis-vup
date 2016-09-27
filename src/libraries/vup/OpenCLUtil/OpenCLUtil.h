// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_OPENCLUTIL_H
#define VUP_OPENCLUTIL_H

// Includes for methods to get context properties are OS-specific.
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

#include "vup/Core/defs.h"
#include "vup/Util/FileReader.h"
#include "vup/Core/datadefs.h"
#include "vup/Exceptions/KernelNotFoundException.h"
#include "vup/Exceptions/KernelCreationException.h"
#include "vup/Exceptions/RunKernelException.h"
#include "vup/Exceptions/CLProgramCompilationException.h"
#include "vup/Exceptions/AcquiringGLObjectsException.h"
#include "vup/Exceptions/ReleasingGLObjectsException.h"
#include "vup/Exceptions/BufferWritingException.h"
#include "vup/Exceptions/BufferNotFoundException.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <functional> 
#include <cctype>
#include <locale>

// This class handles all of OpenCL's methods used within the framework which allows the usage
// of a different GPU programming API by simply creating a different handler.
// The new handler should use the same public functions though, to ensure flawless functionality.

namespace vup {

// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// Trims string from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
    std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// Trims string from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
    std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// Trims string from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

// Removes linebreaks and trims the string
static inline std::string &onelineTrim(std::string &s) {
  s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
  s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
  return trim(s);
}


// Encapsulates OpenCL boilerplate in order to get an OpenCL context and default device to execute kernel code on.
class GPUBoilerplate
{
public:
  // Selects default platform from platformID and looks for device of deviceType with deviceID on this platform.
  // Creates OpenCL context from these parameters specific to the OS.
  GPUBoilerplate(int platformID, cl_device_type deviceType, int deviceID);
  ~GPUBoilerplate();

  cl::Platform getPlatform() { return m_defaultPlatform; }
  cl::Device getDevice() { return m_defaultDevice; }
  cl::Context getContext() { return m_context; }

private:
  cl::Platform m_defaultPlatform;
  cl::Device m_defaultDevice;
  cl::Context m_context;

  std::vector<cl::Platform> m_platforms;
  std::vector<cl::Device> m_devices;
};


// Represents an OpenCL kernel parameter, so data can be assigned correctly
struct KernelArguments {
  std::string name = "";
  int index = 0;
  datatype type = vup::EMPTY;
  bool constant = true;
};

typedef std::map<std::string, std::vector<KernelArguments>> kernelArgumentsMap;
typedef std::map<std::string, cl::Kernel> kernelMap;

// Handles OpenCL program compilation on specified device within a given context.
// Provides functionality to initialize kernels and set kernel arguments.
// Also extracts arguments from kernel function definitions for correct data association.
class KernelHandler
{
public:
  // Builds kernel program on device with given context.
  KernelHandler(cl::Context context, cl::Device device, std::string path);
  // Initializes given list of kernels. 
  KernelHandler(cl::Context context, cl::Device device, std::string path, std::vector<std::string> kernels);
  ~KernelHandler();

  // Reloads kernel program from the same source as previously specified
  // to allow changes in the kernels while the application in running.
  // Kernels that were initialized before the reload are initialized again.
  void reloadProgram();
  // Changes source of kernel program to new path.
  void reloadProgram(std::string path);
  // Changes context and device to build new program on.
  void reloadProgram(cl::Context context, cl::Device device, std::string path);

  // Creates cl::Kernel and adds it to m_kernels with the kernel name as key.
  // Throws KernelCreationException if the kernel creation fails.
  void initKernel(std::string kernel);
  void initKernels(std::vector<std::string> kernels);

  cl::Kernel getKernel(std::string name);

  kernelArgumentsMap getKernelArguments() { return m_arguments; }

  // Sets argument at position index in kernel with given name to data.
  template <class T> void setArg(const char* name, int index, const T &data);
  // Sets arguments of multiple kernels whose names are given to data.
  // The index in all kernels has to be the same.
  template <class T> void setArg(std::vector<std::string> names, int index, const T &data);

private:
  // Builds OpenCL kernel program on device with OS-specific context.
  // In the process the m_program variable is set to the newly built program.
  // Throws CLProgramCompilationException of build fails.
  void buildProgram(cl::Context context, cl::Device device, std::string path, const std::string &src);
  // Extracts kernel arguments from the kernel function declaration in the source code.
  // Arguments of each kernel are saved in an entry in the kernelArgumentsMap m_arguments.
  void extractArguments(std::string path, const std::string &src);
  bool doesKernelExist(std::string name);
  // Splits kernel function parameter strings using the specified split character.
  std::vector<std::string> splitParams(const char* str, char split);
  cl::Context m_context;
  cl::Device m_device;
  cl::Program m_program;
  std::string m_path;
  kernelMap m_kernels;
  kernelArgumentsMap m_arguments;
};

template<class T>
void KernelHandler::setArg(const char* name, int index, const T &data)
{
  if (doesKernelExist(name)) {
    m_kernels[name].setArg(index, data);
  } else
  {
    throw vup::KernelNotFoundException(name);
  }
}
template<class T>
void KernelHandler::setArg(std::vector<std::string> names, int index, const T &data)
{
  for (std::string name : names) {
    setArg(name, index, data);
  }
}

// Encapsulates the OpenCL queue on a specified context. It is preferable to use
// this queue over the OpenCL queue since its methods are less verbose.
class Queue
{
public:
  Queue(cl::Context context);
  ~Queue();
  cl::CommandQueue getQueue() { return m_queue; }
  // Enqueues the writing of data from ptr into the memory of size with an offset of 0 in clBuffer b.
  // This call is blocking.
  void writeBuffer(cl::Buffer b, int size, const void * ptr);
  // Allows to set offset of data. This call is blocking.
  void writeBuffer(cl::Buffer b, int offset, int size, const void * ptr);
  // Allows to set offset of data and if the writing is blocking.
  void writeBuffer(cl::Buffer b, cl_bool blocking, int offset, int size, const void * ptr);
  // Enqueues kernel execution with the specified global range.
  void runRangeKernel(cl::Kernel k, int global);
  // Enqueues kernel execution with the specified global and local range and offset.
  void runRangeKernel(cl::Kernel k, int offset, int global, int local);
  // Enqueues the acquisition of OpenGL memory so it can be used within OpenCL.
  void acquireGL(std::vector<cl::Memory> * mem);
  // Enqueues the release of OpenGL memory from OpenCL so it can be used by OpenGL again.
  void releaseGL(std::vector<cl::Memory> * mem);

  void finish() { m_queue.finish(); }
  void flush() { m_queue.flush(); }

protected:
  cl::CommandQueue m_queue;
  cl::Context m_context;
};

}

#endif
