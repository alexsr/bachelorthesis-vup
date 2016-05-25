#include "vup/defs.h"
#include "vup/init.h"
#include "vup/particle.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/VBOHandler.h"
#include "vup/OpenCLUtil/OpenCLUtil.h"

#include <CL/cl.hpp>
#include <CL/cl_gl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

#define WIDTH 1920
#define HEIGHT 1080

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
  GLFWwindow* window = vup::createWindow(WIDTH, HEIGHT, "Instanced Rendering", nullptr, nullptr);
  vup::initGLEW();

  glViewport(0, 0, WIDTH, HEIGHT);

  glfwSetKeyCallback(window, key_callback);

  vup::TrackballCam cam(WIDTH, HEIGHT, 0.01f, 10.0f, 1.0f);

  vup::ShaderProgram simpleShader(SHADERS_PATH "/instanced.vert", SHADERS_PATH "/instanced.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  // Init particle data
  int particle_amount = 5000;
  std::vector<vup::particle::pos> translations(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    translations[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].w = 1.0f;
  }
  srand(static_cast <unsigned> (time(0)));
  std::vector<vup::particle::color> color(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    color[i].r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].a = 1.0f;
  }
  std::vector<vup::particle::vel> vel(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].w = 0.0f;
  }
  std::vector<vup::particle::type> type(particle_amount);
  
  for (int i = 0; i < particle_amount; i++) {
    type[i] = static_cast<int>(-1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f)));
    
  }

  vup::VBOHandler vboHandler(particle_amount);

  float size = .1f;
  vup::SphereData sphere(size, 30, 30);
  vup::ParticleRenderer renderer(sphere, particle_amount, vboHandler.getPosVBO(), vboHandler.getVBOs());
  vboHandler.updatePositions(&translations);
  vboHandler.updateColor(&color);
  vboHandler.updateType(&type);

  int test = 0;

  // OPENCL
  vup::TBD gpuHandler(1, CL_DEVICE_TYPE_GPU, 0);

  cl_int err;
  cl::Context context = gpuHandler.getContext();
  cl::Device default_device = gpuHandler.getDevice();
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

  // Main loop
  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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