#include "vup/defs.h"
#include "vup/init.h"
#include "vup/particle.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/BufferHandler.h"
#include "vup/ParticleHandling/BufferHandler.cpp"
#include "vup/OpenCLUtil/OpenCLUtil.h"

#include <CL/cl.hpp>
#include <CL/cl_gl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>

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

  int particle_amount = 10000;

  std::vector<glm::vec4> translations(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    translations[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].w = 1.0f;
  }
  srand(static_cast <unsigned> (time(0)));
  std::vector<glm::vec4> color(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    color[i].r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].a = 1.0f;
  }
  std::vector<glm::vec4> vel(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].w = 0.0f;
  }
  std::vector<int> type(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    type[i] = static_cast<int>(-1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f)));

  }

  vup::BufferHandler vboHandler;
  vboHandler.createVBOData("pos", 1, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  vboHandler.createVBOData("color", 2, particle_amount, 4, color, true, GL_STATIC_DRAW);

  float size = .1f;
  vup::SphereData sphere(size, 20, 20);
  vup::ParticleRenderer renderer(sphere, particle_amount, vboHandler.getVBOs());

  int test = 0;

  // OPENCL
  vup::TBD gpuHandler(0, CL_DEVICE_TYPE_GPU, 0);

  cl::Context context = gpuHandler.getContext();
  cl::Device default_device = gpuHandler.getDevice();

  cl::CommandQueue queue(context, default_device);

  vup::FileReader file(OPENCL_KERNEL_PATH "/interop.cl");

  cl::Program::Sources source(1, std::make_pair(file.getSourceChar(), file.length() + 1));

  cl::Program program(context, source);
  if (program.build({ default_device }) != CL_SUCCESS) {
    std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
    exit(-1);
  }
  glFinish();
  cl_int clError;
  cl::BufferGL vbo_cl(context, CL_MEM_READ_WRITE, vboHandler.getInteropVBOHandle("pos"), &clError);
  std::cout << clError << " Error?" << std::endl;
  cl::Buffer vel_cl(context, CL_MEM_READ_ONLY, sizeof(glm::vec4) * vel.size(), NULL, &clError);
  queue.enqueueWriteBuffer(vel_cl, CL_TRUE, 0, sizeof(glm::vec4) * vel.size(), &vel[0]);
  std::cout << clError << " Error?" << std::endl;

  std::vector<cl::Memory> openglbuffers = { vbo_cl };

  cl::Kernel kernel(program, "move", &clError);
  std::cout << clError << " Error?" << std::endl;
  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  int frames = 0;

  kernel.setArg(0, vbo_cl);
  kernel.setArg(1, vel_cl);
  kernel.setArg(2, 0.01f);

  // Main loop
  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    currentTime = glfwGetTime();
    frames++;
    if (currentTime - lastTime >= 1.0) {
      std::ostringstream strs;
      strs << frames;
      std::string title = "FPS: " + strs.str();
      glfwSetWindowTitle(window, title.c_str());
      frames = 0;
      lastTime = currentTime;
    }
    glFinish();
    clError = queue.enqueueAcquireGLObjects(&openglbuffers);
    if (test > 100) {
      test = 0;
      for (int i = 0; i < particle_amount; i++) {
        vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
      }
      queue.enqueueWriteBuffer(vel_cl, CL_TRUE, 0, sizeof(glm::vec4) * vel.size(), &vel[0]);
    }
    clError = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(particle_amount), cl::NullRange);
    clError = queue.enqueueReleaseGLObjects(&openglbuffers);
    queue.finish();
    test++;
    cam.update(window);
    simpleShader.updateUniform("view", cam.getView());
    simpleShader.use();

    renderer.execute(particle_amount);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}