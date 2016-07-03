#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/utils.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/BufferHandler.h"
#include "vup/ParticleHandling/UniformGrid.h"
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

  vup::TrackballCam cam(WIDTH, HEIGHT, 1.0f, 10.0f, 10.0f, glm::vec3(0.0f, 0.0f, 0.0f));
  vup::ShaderProgram simpleShader(SHADERS_PATH "/instanced.vert", SHADERS_PATH "/instanced.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  int particle_amount = 1000;
  vup::position translations(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    translations[i].x = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
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
    particles[i].mass = 18.01528;
    particles[i].density = 1000.0f;
    particles[i].viscosity = 0.890f;
  }

  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::BufferHandler buffers(clBasis.context());
  buffers.createVBOData("pos", 2, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  buffers.createVBOData("color", 3, particle_amount, 4, color, true, GL_STREAM_DRAW);

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
  vup::UniformGrid grid(100, 12, 2.0f, clBasis.context(), CL_MEM_READ_WRITE);
  //queue.writeBuffer(grid.getGridBuffer(), sizeof(int) * grid.getMaxGridCapacity(), &grid.getGridData()[0]);
  //queue.writeBuffer(grid.getCounterBuffer(), sizeof(int) * grid.getCellAmount(), &grid.getCounterData()[0]);
  buffers.createBufferGL("pos_vbo", CL_MEM_READ_WRITE, "pos");
  buffers.createBufferGL("color", CL_MEM_READ_WRITE, "color");
  buffers.createBuffer<vup::velocity>("vel", CL_MEM_READ_WRITE, vel.size());
  queue.writeBuffer(buffers.getBuffer("vel"), sizeof(glm::vec4) * vel.size(), &vel[0]);
  buffers.createBuffer<vup::position>("nextpos", CL_MEM_READ_WRITE, translations.size());
  queue.writeBuffer(buffers.getBuffer("nextpos"), sizeof(glm::vec4) * translations.size(), &translations[0]);
  buffers.createBuffer<vup::particle>("particles", CL_MEM_READ_WRITE, particles.size());
  queue.writeBuffer(buffers.getBuffer("particles"), sizeof(vup::particle) * particles.size(), &particles[0]);
 // queue.setTypeIndices(VUP_FLUID, CL_MEM_READ_WRITE, fluidIndices, particle_amount);
  //queue.setTypeIndices(VUP_RIGID, CL_MEM_READ_WRITE, rigidIndices, particle_amount);
  
  float dt = 0.001f;
  float camdt = 0.01f;
  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();
  vup::KernelHandler kh(clBasis.context(), clBasis.device(), OPENCL_KERNEL_PATH "/sph.cl", {"move", "integrate", "fakecollision", "resetGrid", "updateGrid" });
  
 // kh.setArg("move", 3, queue.getIndexBuffer(VUP_FLUID));
  //kh.initKernel("test");
 // __kernel void move(__global float4* pos, __global float4* next, __global float4* vel, __global particle* particles, __global int* grid, __global int* counter, float cellSize, int lineSize, int cellCapacity, float dt) {
  kh.setArg("move", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("move", 1, buffers.getBuffer("nextpos"));
  kh.setArg("move", 2, buffers.getBuffer("vel"));
  kh.setArg("move", 3, buffers.getBuffer("particles"));
  kh.setArg("move", 4, grid.getGridBuffer());
  kh.setArg("move", 5, grid.getCounterBuffer());
  kh.setArg("move", 6, grid.getGridRadius());
  kh.setArg("move", 7, grid.getCellsPerLine());
  kh.setArg("move", 8, grid.getCellCapacity());
  kh.setArg("move", 9, dt);

  kh.setArg("integrate", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("integrate", 1, buffers.getBuffer("nextpos"));
  kh.setArg("integrate", 2, buffers.getBuffer("vel"));
  kh.setArg("integrate", 3, buffers.getBuffer("particles"));
  kh.setArg("integrate", 4, dt);
 // kh.initKernel("fakecollision");
  kh.setArg("fakecollision", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("fakecollision", 1, buffers.getBuffer("nextpos"));
  kh.setArg("fakecollision", 2, buffers.getBuffer("vel"));
  kh.setArg("fakecollision", 3, dt);
  kh.setArg("fakecollision", 4, particle_amount);
  
  kh.setArg("updateGrid", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("updateGrid", 1, grid.getGridBuffer());
  kh.setArg("updateGrid", 2, grid.getCounterBuffer());
  kh.setArg("updateGrid", 3, grid.getGridRadius());
  kh.setArg("updateGrid", 4, grid.getCellsPerLine());
  kh.setArg("updateGrid", 5, grid.getCellCapacity());

  kh.initKernel("printGrid");
  kh.setArg("printGrid", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("printGrid", 1, grid.getGridBuffer());
  kh.setArg("printGrid", 2, grid.getCounterBuffer());
  kh.setArg("printGrid", 3, grid.getGridRadius());
  kh.setArg("printGrid", 4, grid.getCellsPerLine());
  kh.setArg("printGrid", 5, grid.getCellCapacity());
  kh.setArg("printGrid", 6, buffers.getBufferGL("color"));

  kh.setArg("resetGrid", 0, grid.getCounterBuffer());
  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  double accumulator = 0.0;
  int frames = 0;
  int updates = 200;
  // Main loop
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  while (!glfwWindowShouldClose(window)) {
    vup::clearGL();
    accumulator += glfwGetTime() - currentTime;
    currentTime = glfwGetTime();
    frames++;
    lastTime = vup::updateFramerate(currentTime, lastTime, window, frames);
    // Fixed timestep. Still lagging if rendering is slow. This is intended though
    while (accumulator > dt) {
      accumulator -= dt;
        //  queue.removeIndices(0, std::vector<int>(fluidIndices.begin() + test2, fluidIndices.begin() + test2 + 50));
      queue.acquireGL(&openglbuffers);
      if (updates >= 200) {
        updates = 0;
        queue.runRangeKernel(kh.get("resetGrid"), grid.getCellAmount());
        queue.runRangeKernel(kh.get("updateGrid"), particle_amount);
       // queue.runRangeKernel(kh.get("printGrid"), particle_amount);
      }
      queue.runRangeKernel(kh.get("move"), particle_amount);
      queue.runRangeKernel(kh.get("fakecollision"), particle_amount);
      queue.runRangeKernel(kh.get("integrate"), particle_amount);
      queue.releaseGL(&openglbuffers);
      queue.finish();
      updates++;
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