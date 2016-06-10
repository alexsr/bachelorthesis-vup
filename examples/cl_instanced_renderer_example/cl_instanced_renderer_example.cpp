#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/utils.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/BufferHandler.h"
#include "vup/OpenCLUtil/OpenCLUtil.h"

#include <string>
#include <ctime>

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

  vup::TrackballCam cam(WIDTH, HEIGHT, 1.0f, 10.0f, 10.0f, glm::vec3(0.0f, -4.0f, 0.0f));

  vup::ShaderProgram simpleShader(SHADERS_PATH "/instanced.vert", SHADERS_PATH "/instanced.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  int particle_amount = 500;

  vup::position translations(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    translations[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].w = 1.0f;
  }
  srand(static_cast <unsigned> (time(0)));
  vup::color color(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    color[i].r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].a = 1.0f;
  }
  vup::velocity vel(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    vel[i] = glm::vec4(0.0f);
  }
  vup::type type(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    type[i] = static_cast<int>(-1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f)));
  }
  std::vector<vup::particle> particles(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    particles[i].id = 0;
    particles[i].type = 0;
    particles[i].mass = 5.0f;
    particles[i].density = 3.0f;
    particles[i].viscosity = 3.0f;
  }
  for (int i = 0; i < particle_amount/2; i++) {
    particles[i].id = 0;
    particles[i].type = 0;
    particles[i].mass = 1.0f;
    particles[i].density = 3.0f;
    particles[i].viscosity = 3.0f;
  }

  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::BufferHandler buffers(clBasis.context());
  buffers.createVBOData("pos", 2, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  buffers.createVBOData("color", 3, particle_amount, 4, color, true, GL_STATIC_DRAW);

  float size = .1f;
  vup::SphereData sphere(size, 20, 20);
  vup::ParticleRenderer renderer(sphere, buffers.getInteropVBOs());

  std::vector<int> fluidIndices;
  for (int i = 0; i < 1000; i++) {
    fluidIndices.push_back(i);
  }
  std::vector<int> rigidIndices;
  for (int i = 1000; i < 2000; i++) {
    rigidIndices.push_back(i);
  }

  // OPENCL
  vup::ParticleQueue queue(clBasis.context(), particle_amount);
  buffers.createBufferGL("pos_vbo", CL_MEM_READ_WRITE, "pos");
  buffers.createBuffer<vup::velocity>("vel", CL_MEM_READ_WRITE, vel.size());
  queue.writeBuffer(buffers.getBuffer("vel"), sizeof(glm::vec4) * vel.size(), &vel[0]);
  buffers.createBuffer<vup::particle>("particles", CL_MEM_READ_WRITE, particles.size());
  queue.writeBuffer(buffers.getBuffer("particles"), sizeof(vup::particle) * particles.size(), &particles[0]);
 // queue.setTypeIndices(VUP_FLUID, CL_MEM_READ_WRITE, fluidIndices, particle_amount);
  //queue.setTypeIndices(VUP_RIGID, CL_MEM_READ_WRITE, rigidIndices, particle_amount);
  
  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();

  float dt = 0.01f;
  float camdt = 0.01f;
  vup::KernelHandler kh(clBasis.context(), clBasis.device(), OPENCL_KERNEL_PATH "/fakebox.cl", { "test", "fakecollision" });
  
  kh.initKernel("move");
  kh.setArg("move", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("move", 1, buffers.getBuffer("vel"));
  kh.setArg("move", 2, dt);
 // kh.setArg("move", 3, queue.getIndexBuffer(VUP_FLUID));
  //kh.initKernel("test");
  kh.setArg("test", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("test", 1, buffers.getBuffer("vel"));
  kh.setArg("test", 2, buffers.getBuffer("particles"));
  kh.setArg("test", 3, dt);
 // kh.initKernel("fakecollision");
  kh.setArg("fakecollision", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("fakecollision", 1, buffers.getBuffer("vel"));
  kh.setArg("fakecollision", 2, dt);
  kh.setArg("fakecollision", 3, particle_amount);
  
  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  double accumulator = 0.0;
  int frames = 0;

  // Main loop
  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(window)) {
    vup::clearGL();
    accumulator += glfwGetTime() - currentTime;
    currentTime = glfwGetTime();
    frames++;
    lastTime = vup::updateFramerate(currentTime, lastTime, window, frames);
    int state = glfwGetKey(window, GLFW_KEY_R);
    if (state == GLFW_PRESS) {
      kh.reloadProgram();
      kh.setArg("test", 0, buffers.getBufferGL("pos_vbo"));
      kh.setArg("test", 1, buffers.getBuffer("vel"));
      kh.setArg("test", 2, buffers.getBuffer("particles"));
      kh.setArg("test", 3, dt);
      // kh.initKernel("fakecollision");
      kh.setArg("fakecollision", 0, buffers.getBufferGL("pos_vbo"));
      kh.setArg("fakecollision", 1, buffers.getBuffer("vel"));
      kh.setArg("fakecollision", 2, dt);
      kh.setArg("fakecollision", 3, particle_amount);
    }
    // Fixed timestep. Still lagging if rendering is slow. This is intended though.
    if (accumulator > dt) {
      accumulator = 0.0;
        //  queue.removeIndices(0, std::vector<int>(fluidIndices.begin() + test2, fluidIndices.begin() + test2 + 50));
      queue.acquireGL(&openglbuffers);
      queue.runRangeKernel(kh.get("fakecollision"), particle_amount);
      queue.runRangeKernel(kh.get("test"), particle_amount);
      queue.releaseGL(&openglbuffers);
      queue.finish();
    }
    cam.update(window, camdt);
    simpleShader.updateUniform("view", cam.getView());
    simpleShader.use();

    renderer.execute(particle_amount);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}