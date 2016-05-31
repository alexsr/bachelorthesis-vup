#include "OpenCLUtil.h"

vup::OpenCLBasis::OpenCLBasis(int platformID, cl_device_type deviceType, int deviceID)
{
  initPlatform(platformID);
  initContext(deviceType);
  initDevice(deviceID);
}

vup::OpenCLBasis::~OpenCLBasis()
{
}

void vup::OpenCLBasis::setDefaultPlatform(int id)
{
  if (m_platforms.size() > id) {
    m_defaultPlatform = m_platforms[id];
  }
  else {
    std::cout << "Platform " << id << " not found. Using Platform 0.\n";
    m_defaultPlatform = m_platforms[0];
  }

  std::cout << "Using platform: " << m_defaultPlatform.getInfo<CL_PLATFORM_NAME>() << "\n";
}

void vup::OpenCLBasis::setDefaultDevice(int id)
{
  m_defaultDevice = m_devices[id];
  std::cout << "Using device: " << m_defaultDevice.getInfo<CL_DEVICE_NAME>() << "\n";
}

void vup::OpenCLBasis::initPlatform(int id)
{
  cl::Platform::get(&m_platforms);
  if (m_platforms.size() == 0) {
    throw std::exception("No platform found.");
  }
  setDefaultPlatform(id);
}

void vup::OpenCLBasis::initContext(cl_device_type t)
{
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

  m_context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
}

void vup::OpenCLBasis::initDevice(int id)
{
  m_devices = m_context.getInfo<CL_CONTEXT_DEVICES>();
  setDefaultDevice(id);
}

vup::KernelRunner::KernelRunner(cl::Context context, cl::Device device, const char * programPath)
{
  m_queue = cl::CommandQueue(context, device);
  vup::FileReader file(OPENCL_KERNEL_PATH "/interop.cl");
  m_program = cl::Program(context, file.getSourceChar());
  if (m_program.build({ device }) != CL_SUCCESS) {
    throw vup::CLProgramCompilationException(std::string(programPath));
  }
  m_kernels = std::map<std::string, cl::Kernel>();
}

vup::KernelRunner::~KernelRunner()
{
}

void vup::KernelRunner::add(std::string name)
{
  if (doesKernelExist(name))
  {
    std::cout << "WARNING: Kernel " << name << "already exists.";
  }
  cl_int clError;
  m_kernels[name] = cl::Kernel(m_program, name.c_str(), &clError);
  if (clError != CL_SUCCESS) {
    throw vup::KernelCreationException(name, clError);
  }
}

cl::Kernel vup::KernelRunner::get(std::string name)
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

void vup::KernelRunner::writeBuffer(cl::Buffer b, cl_bool blocking, int offset, int size, const void * ptr)
{
  cl_int clError =  m_queue.enqueueWriteBuffer(b, blocking, offset, size, ptr);
  if (clError != CL_SUCCESS) {
    throw vup::BufferWritingException(clError);
  }
}

void vup::KernelRunner::runRangeKernel(std::string name, cl::NDRange offset, cl::NDRange global, cl::NDRange local)
{
  if (!doesKernelExist(name)) {
    throw vup::KernelNotFoundException(name);
  }    
  cl_int clError = m_queue.enqueueNDRangeKernel(m_kernels[name], offset, global, local);
  if (clError != CL_SUCCESS) {
    throw vup::RunKernelException(name, clError);
  }
}

void vup::KernelRunner::acquireGL(std::vector<cl::Memory>* mem)
{
  glFinish();
  cl_int clError = m_queue.enqueueAcquireGLObjects(mem);
  if (clError != CL_SUCCESS) {
    throw vup::AcquiringGLObjectsException(clError);
  }
}

void vup::KernelRunner::releaseGL(std::vector<cl::Memory>* mem)
{
  cl_int clError = m_queue.enqueueReleaseGLObjects(mem);
  if (clError != CL_SUCCESS) {
    throw vup::ReleasingGLObjectsException(clError);
  }
}

bool vup::KernelRunner::doesKernelExist(std::string name)
{
  std::map<std::string, cl::Kernel>::iterator it = m_kernels.find(name);
  if (it != m_kernels.end())
  {
    return true;
  }
  return false;
}
