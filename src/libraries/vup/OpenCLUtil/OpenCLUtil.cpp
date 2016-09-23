#include "OpenCLUtil.h"

// OpenCLBoilerplate

vup::GPUBoilerplate::GPUBoilerplate(int platformID, cl_device_type deviceType, int deviceID)
{
  cl::Platform::get(&m_platforms);
  if (m_platforms.size() == 0) {
    throw std::exception("No platform found.");
  }
  else if (m_platforms.size() <= platformID) {
    std::cout << "Platform " << platformID << " not found." << std::endl;
    platformID = 0;
  }
  m_defaultPlatform = m_platforms[platformID];
  std::cout << "Using platform: " << m_defaultPlatform.getInfo<CL_PLATFORM_NAME>() << std::endl;


  m_defaultPlatform.getDevices(deviceType, &m_devices);
  if (m_devices.size() == 0) {
    throw std::exception("No device found.");
  }
  else if (m_devices.size() <= deviceID) {
    std::cout << "Device " << deviceID << " not found on platform " << platformID << "." << std::endl;
    deviceID = 0;
  }
  m_defaultDevice = m_devices[deviceID];
  std::cout << "Using device: " << m_defaultDevice.getInfo<CL_DEVICE_NAME>() << std::endl;

  // The context properties are OS-specific. Therefore the OS has to be checked here.
#if defined (__APPLE__)
  CGLContextObj kCGLContext = CGLGetCurrentContext();
  CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
  cl_context_properties properties[] =
  {
    CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
    0
  };
#else
#ifdef UNIX
  cl_context_properties properties[] =
  {
    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
    CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform,
    0
  };
#else // Win32
  cl_context_properties properties[] =
  {
    CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM, (cl_context_properties)m_defaultPlatform(), // OpenCL platform object
    0
  };
#endif
#endif
  m_context = cl::Context(m_defaultDevice, properties);
}

vup::GPUBoilerplate::~GPUBoilerplate()
{
}


// KernelHandler

vup::KernelHandler::KernelHandler(cl::Context context, cl::Device device, std::string path)
{
  m_context = context;
  m_device = device;

  m_path = path;
  vup::FileReader file(path);
  std::string source = file.getSource();

  buildProgram(context, device, m_path, source);
  extractArguments(path, source);

  m_kernels = kernelMap();
}
vup::KernelHandler::KernelHandler(cl::Context context, cl::Device device, std::string path, std::vector<std::string> kernels) : KernelHandler(context, device, path)
{
  initKernels(kernels);
}
vup::KernelHandler::~KernelHandler()
{
}
void vup::KernelHandler::reloadProgram()
{
  vup::FileReader file(m_path);
  buildProgram(m_context, m_device, m_path, file.getSource());
  for (kernelMap::iterator it = m_kernels.begin(); it != m_kernels.end(); ++it) {
    initKernel(it->first);
  }
}
void vup::KernelHandler::reloadProgram(std::string path)
{
  m_path = path;
  reloadProgram();
}
void vup::KernelHandler::reloadProgram(cl::Context context, cl::Device device, std::string path)
{
  m_context = context;
  m_device = device;
  reloadProgram(path);
}
void vup::KernelHandler::initKernels(std::vector<std::string> kernels)
{
  for (unsigned int i = 0; i < kernels.size(); i++) {
    initKernel(kernels.at(i));
  }
}
void vup::KernelHandler::initKernel(std::string kernel)
{
  cl_int clError;
  m_kernels[kernel] = cl::Kernel(m_program, kernel.c_str(), &clError);
  if (clError != CL_SUCCESS) {
    throw vup::KernelCreationException(kernel, clError);
  }
 }

cl::Kernel vup::KernelHandler::getKernel(std::string name)
{
  if (doesKernelExist(name))
  {
    return m_kernels[name];
  }
  else
  {
    throw vup::KernelNotFoundException(name);
  }
}

void vup::KernelHandler::buildProgram(cl::Context context, cl::Device device, std::string path, const std::string &src)
{
  m_program = cl::Program(context, src.c_str());
  std::vector<cl::Device> deviceVector = { device };
  cl_int clError = m_program.build(deviceVector);
  if (clError != CL_SUCCESS) {
    std::string buildinfo = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    throw vup::CLProgramCompilationException(std::string(path), buildinfo);
  }
}

void vup::KernelHandler::extractArguments(std::string path, const std::string &src)
{
  int kernelPos = 0;
  // Loop through the source and look for kernel functions until there are no more behind kernelPos.
  // The definition of the kernel function is then parsed and its name and arguments are stored.
  // This sets the kernelPos to the position in the src where a kernel definition is found.
  while ((kernelPos = (src.find("__kernel void", kernelPos))) != std::string::npos) {
    // skip 14 characters to get to the name of the kernel
    int namePos = kernelPos + 14;
    int endOfName = src.find("(", namePos);
    std::string kernelName = onelineTrim(src.substr(namePos, endOfName - namePos));
    m_arguments[kernelName] = std::vector<KernelArguments>();

    int endOfParams = src.find(")", endOfName);
    kernelPos = endOfParams;
    std::string paramStr = onelineTrim(src.substr(endOfName + 1, endOfParams - endOfName - 1));
    std::vector<std::string> params = splitParams(paramStr.c_str(), ',');

    // Index is the position of the argument in the function declaration.
    int index = 0;
    // Loop through all parameter strings and split them into their parts.
    // Local or constant memory is not handled differently from global memory.
    for (auto &param : params) {
      std::vector<std::string> parts = splitParams(param.c_str(), ' ');
      if (parts.size() < 2) {
        throw new std::exception(); // TODO: change this exception
      }
      KernelArguments karg;
      // The kernel name always is the last string in the parameter string.
      std::string name = parts.at(parts.size()-1);
      datatype type = vup::EMPTY;
      // The kernel name always is the second to last string in the parameter string.
      std::string typestring = parts.at(parts.size()-2);
      // If the type string contains a * the argument refers to a set of data, therefore it is not a constant
      if (typestring.find("*") != std::string::npos) {
        karg.constant = false;
      }
      typestring.erase(std::remove(typestring.begin(), typestring.end(), '*'), typestring.end());
      if (typestring == "float") {
        type = vup::FLOAT;
      }
      else if (typestring == "int") {
        type = vup::INT;
      }
      else if (typestring == "float4") {
        type = vup::VEC4;
      }
      karg.type = type;
      karg.index = index;
      karg.name = name;
      index++;
      m_arguments[kernelName].push_back(karg);
    }
  }
}

bool vup::KernelHandler::doesKernelExist(std::string name)
{
  kernelMap::iterator it = m_kernels.find(name);
  return it != m_kernels.end();
}

std::vector<std::string> vup::KernelHandler::splitParams(const char * str, char split)
{
  std::vector<std::string> params;
  // Loop through the string until the pointer to the current char is a nullpointer.
  do {
    const char *begin = str;
    while (*str != split && *str){
      str++;
    }
    params.push_back(vup::trim(std::string(begin, str)));
    // Skip multiple consecutive occurrences of the split character.
    while (*str == split && *str) {
      str++;
    }
  } while (0 != *str);
  return params;
}


// OpenCL Queue

vup::Queue::Queue(cl::Context context)
{
  m_context = context;
  m_queue = cl::CommandQueue(context);
}

vup::Queue::~Queue()
{
}

void vup::Queue::writeBuffer(cl::Buffer b, int size, const void * ptr)
{
  cl_int clError = m_queue.enqueueWriteBuffer(b, CL_TRUE, 0, size, ptr);
  finish();
  if (clError != CL_SUCCESS) {
    throw vup::BufferWritingException(clError);
  }
}

void vup::Queue::writeBuffer(cl::Buffer b, int offset, int size, const void * ptr)
{
  cl_int clError = m_queue.enqueueWriteBuffer(b, CL_TRUE, offset, size, ptr);
  finish();
  if (clError != CL_SUCCESS) {
    throw vup::BufferWritingException(clError);
  }
}

void vup::Queue::writeBuffer(cl::Buffer b, cl_bool blocking, int offset, int size, const void * ptr)
{
  cl_int clError = m_queue.enqueueWriteBuffer(b, blocking, offset, size, ptr);
  finish();
  if (clError != CL_SUCCESS) {
    throw vup::BufferWritingException(clError);
  }
}

void vup::Queue::runRangeKernel(cl::Kernel k, int global)
{
  cl_int clError = m_queue.enqueueNDRangeKernel(k, cl::NullRange, cl::NDRange(global), cl::NullRange);
  if (clError != CL_SUCCESS) {
    throw vup::RunKernelException(clError);
  }
}

void vup::Queue::runRangeKernel(cl::Kernel k, int offset, int global, int local)
{
  cl_int clError = m_queue.enqueueNDRangeKernel(k, cl::NDRange(offset), cl::NDRange(global), cl::NDRange(local));
  if (clError != CL_SUCCESS) {
    throw vup::RunKernelException(clError);
  }
}

void vup::Queue::acquireGL(std::vector<cl::Memory>* mem)
{
  // Call glFinish to ensure all OpenGL calls are done.
  glFinish();
  cl_int clError = m_queue.enqueueAcquireGLObjects(mem);
  if (clError != CL_SUCCESS) {
    throw vup::AcquiringGLObjectsException(clError);
  }
}

void vup::Queue::releaseGL(std::vector<cl::Memory>* mem)
{
  cl_int clError = m_queue.enqueueReleaseGLObjects(mem);
  finish();
  // Call finish to ensure OpenCL is done working on OpenGL data.
  if (clError != CL_SUCCESS) {
    throw vup::ReleasingGLObjectsException(clError);
  }
}
