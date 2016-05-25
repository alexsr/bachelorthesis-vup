#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/VBOHandler.h"
#include <Windows.h>

#include <CL/cl.hpp>
#include <CL/cl_gl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

#define WIDTH 800
#define HEIGHT 600

#if defined (__APPLE__) || defined(MACOSX)
#define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

inline void checkErr(cl_int err, const char * name) {
  if (err != CL_SUCCESS) {
    std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main()
{
  // GLFW
  GLint glfwError = glfwInit();
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Instanced Rendering", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwMakeContextCurrent(window);

  // GLEW
  GLenum glewError = glewInit();
  if (GLEW_OK != glewError) {
    std::cout << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
    return -1;
  }

  glViewport(0, 0, WIDTH, HEIGHT);

  glfwSetKeyCallback(window, key_callback);

  vup::TrackballCam cam(WIDTH, HEIGHT, 0.01f, 100.0f, 5.0f);

  vup::ShaderProgram simpleShader(SHADERS_PATH "/instanced.vert", SHADERS_PATH "/instanced.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  int particle_amount = 50000;

  std::vector<vup::particle::vel> vel(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].w = 0.0f;
  }

  std::vector<vup::particle::pos> translations(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    translations[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].w = 1.0f;
  }
  std::vector<vup::particle::color> color(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    color[i].r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].a = 1.0f;
  }

  std::vector<vup::particle::type> type(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    type[i] = static_cast<int>(-1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f)));
    
  }

  vup::VBOHandler vboHandler(particle_amount);

  float size = .01f;
  vup::SphereData sphere(size, 20, 20);
  vup::ParticleRenderer renderer(sphere, particle_amount, vboHandler.getPosVBO(), vboHandler.getVBOs());
  vboHandler.updatePositions(&translations);
  vboHandler.updateColor(&color);
  vboHandler.updateType(&type);

  int test = 0;

  // OPENCL

  std::vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);
  if (all_platforms.size() == 0) {
    std::cout << " No platforms found. Check OpenCL installation!\n";
    exit(1);
  }
  cl::Platform default_platform = all_platforms[1];
  std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

  std::vector<cl::Device> all_devices;
  default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
  if (all_devices.size() == 0) {
    std::cout << " No devices found. Check OpenCL installation!\n";
    exit(1);
  }
  cl_context_properties properties[] =
  {
    CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM, (cl_context_properties)default_platform(), // OpenCL platform object
    0
  };
  cl_int err;

  cl::Context context(CL_DEVICE_TYPE_GPU, properties);
  std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
  cl::Device default_device = devices[0];
  std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

  cl::CommandQueue queue(context, default_device);

  vup::FileReader file(OPENCL_KERNEL_PATH "/interop.cl");

  cl::Program::Sources source(1, std::make_pair(file.getSourceChar(), file.length() + 1));

  cl::Program program(context, source);
  if (program.build({ default_device }) != CL_SUCCESS) {
    std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
    exit(1);
  }
  glFinish();
  cl_int clError;
  cl::BufferGL vbo_cl(context, CL_MEM_READ_WRITE, vboHandler.getPosVBO(), &clError);
  std::cout << clError << " Error?" << std::endl;
  cl::Buffer vel_cl(context, CL_MEM_READ_ONLY, sizeof(vup::particle::vel) * vel.size(), NULL, &clError);
  queue.enqueueWriteBuffer(vel_cl, CL_TRUE, 0, sizeof(vup::particle::vel) * vel.size(), &vel[0]);
  std::cout << clError << " Error?" << std::endl;

  std::vector<cl::Memory> openglbuffers = { vbo_cl };

  cl::Kernel kernel(program, "move", &clError);
  std::cout << clError << " Error?" << std::endl;
  kernel.setArg(0, vbo_cl);
  kernel.setArg(1, vel_cl);



  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /*for (int i = 0; i < 1000; i++) {
      translations[i].x += vel[i].x * 0.01f;
      translations[i].y += vel[i].y * 0.01f;
      translations[i].z += vel[i].z * 0.01f;
    }*/
    cam.update(window);
    simpleShader.updateUniform("view", cam.getView());
    simpleShader.use();
    glFinish();
    clError = queue.enqueueAcquireGLObjects(&openglbuffers);
    if (test > 100) {
      test = 0;
      for (int i = 0; i < particle_amount; i++) {
        vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
      }
      queue.enqueueWriteBuffer(vel_cl, CL_TRUE, 0, sizeof(vup::particle::vel) * vel.size(), &vel[0]);
    }
    clError = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(particle_amount), cl::NullRange);
    clError = queue.enqueueReleaseGLObjects(&openglbuffers);
    queue.finish();
    renderer.execute(particle_amount);
    glfwPollEvents();
    glfwSwapBuffers(window);
    test++;
  }
  glfwTerminate();
  return 0;
}