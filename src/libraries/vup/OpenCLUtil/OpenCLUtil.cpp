#include "OpenCLUtil.h"

vup::TBD::TBD(int platformID, cl_device_type deviceType, int deviceID)
{
  initPlatform(platformID);
  initContext(deviceType);
  initDevice(deviceID);
  m_queue = cl::CommandQueue(m_context, m_defaultDevice);
}

vup::TBD::~TBD()
{
}

void vup::TBD::setDefaultPlatform(int id)
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

void vup::TBD::setDefaultDevice(int id)
{
  m_defaultDevice = m_devices[id];
  std::cout << "Using device: " << m_defaultDevice.getInfo<CL_DEVICE_NAME>() << "\n";
}

void vup::TBD::initPlatform(int id)
{
  cl::Platform::get(&m_platforms);
  if (m_platforms.size() == 0) {
    std::cout << " No platforms found. Check OpenCL installation!\n";
    throw std::exception();
  }
  setDefaultPlatform(id);
}

void vup::TBD::initContext(cl_device_type t)
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

void vup::TBD::initDevice(int id)
{
  m_devices = m_context.getInfo<CL_CONTEXT_DEVICES>();
  setDefaultDevice(id);
}
