#include "OpenCLUtil.h"

vup::OpenCLBasis::OpenCLBasis(int platformID, cl_device_type deviceType, int deviceID)
{
  cl::Platform::get(&m_platforms);
  if (m_platforms.size() == 0) {
    throw std::exception("No platform found.");
  }
  if (m_platforms.size() > platformID) {
    m_defaultPlatform = m_platforms[platformID];
  }
  else {
    std::cout << "Platform " << platformID << " not found. Using Platform 0.\n";
    m_defaultPlatform = m_platforms[0];
  }
  std::cout << "Using platform: " << m_defaultPlatform.getInfo<CL_PLATFORM_NAME>() << "\n";
  m_defaultPlatform.getDevices(deviceType, &m_devices);
  if (m_devices.size() == 0) {
    throw std::exception("No device found.");
  }
  m_defaultDevice = m_devices[deviceID];
  std::cout << "Using device: " << m_defaultDevice.getInfo<CL_DEVICE_NAME>() << "\n";
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

vup::OpenCLBasis::~OpenCLBasis()
{
}

vup::KernelHandler::KernelHandler(cl::Context context, cl::Device device, const char * path)
{
  buildProgram(context, device, path);
  m_kernels = std::map<std::string, cl::Kernel>();
}
vup::KernelHandler::KernelHandler(cl::Context context, cl::Device device, const char * path, std::vector<std::string> kernels)
{
  buildProgram(context, device, path);
  initKernels(kernels);
}
vup::KernelHandler::~KernelHandler()
{
}
void vup::KernelHandler::initKernels(std::vector<std::string> kernels)
{
  for (int i = 0; i < kernels.size(); i++) {
    initKernel(kernels.at(i));
  }
}
void vup::KernelHandler::initKernel(std::string kernel)
{
  cl_int clError;
  m_kernels.emplace(kernel, cl::Kernel(m_program, kernel.c_str(), &clError));
  if (clError != CL_SUCCESS) {
    throw vup::KernelCreationException(kernel, clError);
  }
}

cl::Kernel vup::KernelHandler::get(std::string name)
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

void vup::KernelHandler::buildProgram(cl::Context context, cl::Device device, const char * path)
{
  vup::FileReader file(path);
  m_program = cl::Program(context, file.getSourceChar());
  cl_int clError = m_program.build({ device });
  if (clError != CL_SUCCESS) {
    std::string buildinfo = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    throw vup::CLProgramCompilationException(std::string(path), buildinfo);
  }
}

bool vup::KernelHandler::doesKernelExist(std::string name)
{
  std::map<std::string, cl::Kernel>::iterator it = m_kernels.find(name);
  if (it != m_kernels.end())
  {
    return true;
  }
  return false;
}

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
  if (clError != CL_SUCCESS) {
    throw vup::BufferWritingException(clError);
  }
}

void vup::Queue::writeBuffer(cl::Buffer b, cl_bool blocking, int offset, int size, const void * ptr)
{
  cl_int clError = m_queue.enqueueWriteBuffer(b, blocking, offset, size, ptr);
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
  glFinish();
  cl_int clError = m_queue.enqueueAcquireGLObjects(mem);
  if (clError != CL_SUCCESS) {
    throw vup::AcquiringGLObjectsException(clError);
  }
}

void vup::Queue::releaseGL(std::vector<cl::Memory>* mem)
{
  cl_int clError = m_queue.enqueueReleaseGLObjects(mem);
  if (clError != CL_SUCCESS) {
    throw vup::ReleasingGLObjectsException(clError);
  }
}


// ParticleQueue
vup::ParticleQueue::ParticleQueue(cl::Context context, int particleAmount) : vup::Queue(context)
{
  m_particleAmount = particleAmount;
}

vup::ParticleQueue::~ParticleQueue()
{
}

void vup::ParticleQueue::runKernelOnType(cl::Kernel k, int type)
{
  if (m_typeIndices.at(type).range() != 0) {
    runRangeKernel(k, m_typeIndices.at(type).range());
  }
}

void vup::ParticleQueue::setTypeIndices(int type, cl_mem_flags flags, std::vector<int> indices, cl_bool blocking)
{
  std::sort(indices.begin(), indices.end());
  indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
  m_typeIndices[type] = vup::TypeBuffer(indices, flags, blocking, m_context, m_particleAmount);
  writeBuffer(m_typeIndices.at(type).buffer(), m_typeIndices.at(type).size(), m_typeIndices.at(type).getIndexPointer(type));
}

std::vector<int> vup::ParticleQueue::getIndices(int type)
{
  if (doesTypeExist(type)) {
    return m_typeIndices.at(type).indices();
  }
  return std::vector<int>(0);
}

int vup::ParticleQueue::getIndicesAmount(int type)
{
  return m_typeIndices.at(type).range();
}

cl::Buffer vup::ParticleQueue::getIndexBuffer(int type)
{
  if (doesTypeExist(type)) {
    return m_typeIndices.at(type).buffer();
  }
  else
  {
    throw vup::BufferNotFoundException(std::to_string(type));
  }
}

void vup::ParticleQueue::addIndices(int type, std::vector<int> indices)
{
  if (doesTypeExist(type)) {
    m_typeIndices.at(type).addIndices(indices);
    writeBuffer(m_typeIndices.at(type).buffer(), m_typeIndices.at(type).size(), m_typeIndices.at(type).getIndexPointer(type));
  }
}

void vup::ParticleQueue::removeIndices(int type, std::vector<int> indices)
{
  if (doesTypeExist(type)) {
    m_typeIndices.at(type).removeIndices(indices);
    if (m_typeIndices.at(type).range() != 0) {
      writeBuffer(m_typeIndices.at(type).buffer(), m_typeIndices.at(type).size(), m_typeIndices.at(type).getIndexPointer(type));
    }
  }
}

bool vup::ParticleQueue::doesTypeExist(int type)
{
  std::map<int, vup::TypeBuffer>::iterator it = m_typeIndices.find(type);
  if (it != m_typeIndices.end())
  {
    return true;
  }
  return false;
}

