#include "vup/defs.h"
#include "vup/init.h"
#include "vup/particle.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/VBOHandler.h"
#include "vup/ParticleHandling/BufferHandler.h"
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

  vup::VBOHandler vbos;
  vbos.createVBOData("pos", 1, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  vbos.createVBOData("color", 2, particle_amount, 4, color, true, GL_STATIC_DRAW);

  float size = .1f;
  vup::SphereData sphere(size, 20, 20);
  vup::ParticleRenderer renderer(sphere, particle_amount, vbos.getVBOs());

  int test = 0;

  // OPENCL
  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::KernelRunner queue(clBasis.getContext(), clBasis.getDevice(), OPENCL_KERNEL_PATH "/interop.cl");
  //  cl::CommandQueue queue2(queue.getQueue());
  vup::BufferHandler buffers(clBasis.getContext());
  cl_int clError;
  buffers.addGL("pos_vbo", CL_MEM_READ_WRITE, vbos.getInteropVBOHandle("pos"));
  buffers.add("vel", CL_MEM_READ_ONLY, sizeof(glm::vec4) * vel.size());
  queue.writeBuffer(buffers.get("vel"), CL_TRUE, 0, sizeof(glm::vec4) * vel.size(), &vel[0]);

  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();

  queue.add("move");

  queue.setArg("move", 0, buffers.getGL("pos_vbo"));
  queue.setArg("move", 1, buffers.get("vel"));
  queue.setArg("move", 2, 0.01f);

  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  int frames = 0;

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
    if (test > 100) {
      test = 0;
      for (int i = 0; i < particle_amount; i++) {
        vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
      }
      queue.writeBuffer(buffers.get("vel"), CL_TRUE, 0, sizeof(glm::vec4) * vel.size(), &vel[0]);
    }
    queue.acquireGL(&openglbuffers);
    queue.runRangeKernel("move", cl::NullRange, cl::NDRange(particle_amount), cl::NullRange);
    queue.releaseGL(&openglbuffers);
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