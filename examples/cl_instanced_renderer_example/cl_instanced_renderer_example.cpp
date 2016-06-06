#include "vup/defs.h"
#include "vup/init.h"
#include "vup/particle.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
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

int main()
{
  GLFWwindow* window = vup::createWindow(WIDTH, HEIGHT, "Instanced Rendering", nullptr, nullptr);
  glfwSetKeyCallback(window, key_callback);

  vup::initGLEW();
  glViewport(0, 0, WIDTH, HEIGHT);

  vup::TrackballCam cam(WIDTH, HEIGHT, 1.0f, 10.0f, 10.0f);

  vup::ShaderProgram simpleShader(SHADERS_PATH "/instanced.vert", SHADERS_PATH "/instanced.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  int particle_amount = 1000;

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

  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::BufferHandler buffers(clBasis.context());
  buffers.createVBOData("pos", 1, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  buffers.createVBOData("color", 2, particle_amount, 4, color, true, GL_STATIC_DRAW);

  float size = .1f;
  vup::SphereData sphere(size, 20, 20);
  vup::ParticleRenderer renderer(sphere, particle_amount, buffers.getInteropVBOs());

  std::vector<int> fluidIndices;
  for (int i = 0; i < 1000; i++) {
    fluidIndices.push_back(i);
  }

  // OPENCL
  cl::CommandQueue queue2(clBasis.context());
  vup::ParticleQueue queue(clBasis.context(), particle_amount);
  buffers.addGL("pos_vbo", CL_MEM_READ_WRITE, "pos");
  buffers.add("vel", CL_MEM_READ_ONLY, sizeof(glm::vec4) * vel.size());
  queue.writeBuffer(buffers.get("vel"), sizeof(glm::vec4) * vel.size(), &vel[0]);
  queue.setTypeIndices(0, CL_MEM_READ_WRITE, fluidIndices, particle_amount);
  
  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();

  float dt = 0.01f;
  vup::KernelHandler kh(clBasis.context(), OPENCL_KERNEL_PATH "/interop.cl");
  kh.initKernel("move");
  kh.setArg("move", 0, buffers.getGL("pos_vbo"));
  kh.setArg("move", 1, buffers.get("vel"));
  kh.setArg("move", 2, dt);
  kh.setArg("move", 3, queue.getIndexBuffer(0));
  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  double accumulator = 0.0;
  int frames = 0;
  int test = 0;
  int test2 = 0;

  // Main loop
  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    accumulator += glfwGetTime() - currentTime;
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
    if (accumulator > dt) {
      accumulator = 0.0;
      if (test > 100) {
        test = 0;
        for (int i = 0; i < particle_amount; i++) {
          vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
          vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
          vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        }
        queue.writeBuffer(buffers.get("vel"), sizeof(glm::vec4) * vel.size(), &vel[0]);
        if (test2 + 50 <= 1000) {
          queue.removeIndices(0, std::vector<int>(fluidIndices.begin() + test2, fluidIndices.begin() + test2 + 50));
        }
        test2 += 50;
      }
      queue.acquireGL(&openglbuffers);
      queue.runKernelOnType(kh.get("move"), 0);
      queue.releaseGL(&openglbuffers);
      queue.finish();
      test++;
      cam.update(window, dt);
    }
    simpleShader.updateUniform("view", cam.getView());
    simpleShader.use();

    renderer.execute(particle_amount);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}