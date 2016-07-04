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

  int particle_amount = 100;
  std::vector<glm::vec4> translations(particle_amount);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < particle_amount; i++) {
    translations[i].x = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
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
  std::vector<int> type(particle_amount);
  std::vector<float> mass(particle_amount);
  std::vector<float> density(particle_amount);
  std::vector<float> pressure(particle_amount);
  std::vector<glm::vec4> force(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    vel[i] = glm::vec4(0.0f);
    type[i] = static_cast<int>(-1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f)));
    mass[i] = 0.000055f;
    density[i] = 0.59f;
    pressure[i] = 0.0f;
    force[i] = glm::vec4(0.0f);
  }

  int neighbor_amount = 500;
  std::vector<int> neighborCounter(particle_amount);
  for (int i = 0; i < neighborCounter.size(); i++) {
    neighborCounter[i] = 0;
  }
  std::vector<int> neighbors(particle_amount * neighbor_amount);
  for (int i = 0; i < neighbors.size(); i++) {
    neighbors[i] = 0;
  }

  float size = .1f;
  vup::SphereData sphere(size, 20, 20);
  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::BufferHandler buffers(clBasis.context());
  buffers.createVBOData("pos", 2, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  buffers.createVBOData("color", 3, particle_amount, 4, color, true, GL_STREAM_DRAW);
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
  vup::UniformGrid grid(20, 10, 12.0f, clBasis.context(), CL_MEM_READ_WRITE);
  queue.writeBuffer(grid.getGridBuffer(), sizeof(int) * grid.getMaxGridCapacity(), &grid.getGridData()[0]);
  queue.writeBuffer(grid.getCounterBuffer(), sizeof(int) * grid.getCellAmount(), &grid.getCounterData()[0]);
  buffers.createBufferGL("pos_vbo", CL_MEM_READ_WRITE, "pos");
  buffers.createBufferGL("color", CL_MEM_READ_WRITE, "color");
  buffers.createBuffer<glm::vec4>("vel", CL_MEM_READ_WRITE, vel.size());
  queue.writeBuffer(buffers.getBuffer("vel"), sizeof(glm::vec4) * vel.size(), &vel[0]);
  buffers.createBuffer<glm::vec4>("force", CL_MEM_READ_WRITE, force.size());
  queue.writeBuffer(buffers.getBuffer("force"), sizeof(glm::vec4) * force.size(), &force[0]);
  buffers.createBuffer<float>("mass", CL_MEM_READ_WRITE, mass.size());
  queue.writeBuffer(buffers.getBuffer("mass"), sizeof(float) * mass.size(), &mass[0]);
  buffers.createBuffer<float>("pressure", CL_MEM_READ_WRITE, pressure.size());
  queue.writeBuffer(buffers.getBuffer("pressure"), sizeof(float) * pressure.size(), &pressure[0]);
  buffers.createBuffer<float>("density", CL_MEM_READ_WRITE, density.size());
  queue.writeBuffer(buffers.getBuffer("density"), sizeof(float) * density.size(), &density[0]);
  buffers.createBuffer<float>("mass", CL_MEM_READ_WRITE, mass.size());
  queue.writeBuffer(buffers.getBuffer("mass"), sizeof(float) * mass.size(), &mass[0]);
  buffers.createBuffer<int>("neighbors", CL_MEM_READ_WRITE, neighbors.size());
  queue.writeBuffer(buffers.getBuffer("neighbors"), sizeof(int) * neighbors.size(), &neighbors[0]);
  buffers.createBuffer<int>("neighborCounter", CL_MEM_READ_WRITE, neighborCounter.size());
  queue.writeBuffer(buffers.getBuffer("neighborCounter"), sizeof(int) * neighborCounter.size(), &neighborCounter[0]);
 // queue.setTypeIndices(VUP_FLUID, CL_MEM_READ_WRITE, fluidIndices, particle_amount);
  //queue.setTypeIndices(VUP_RIGID, CL_MEM_READ_WRITE, rigidIndices, particle_amount);
  
  float dt = 0.001f;
  float camdt = 0.01f;
  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();
  vup::KernelHandler kh(clBasis.context(), clBasis.device(), OPENCL_KERNEL_PATH "/sph_force.cl", {"integration", "SPH", "densityPressureCalc", "neighbours" });
  
 // kh.setArg("move", 3, queue.getIndexBuffer(VUP_FLUID));
  //kh.initKernel("test");
 // __kernel void move(__global float4* pos, __global float4* next, __global float4* vel, __global particle* particles, __global int* grid, __global int* counter, float cellSize, int lineSize, int cellCapacity, float dt) {

  float smoothingLength = 0.2f;
  float xleft = -1.0f;
/*  __kernel void SPH(__global float4* pos, __global float4* vel, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure,
    __global float* mass, __global float4* forceIntern, float smoothingLength, float neighbor_amount)*/
 
  kh.setArg("SPH", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("SPH", 1, buffers.getBuffer("vel"));
  kh.setArg("SPH", 2, buffers.getBuffer("neighbors"));
  kh.setArg("SPH", 3, buffers.getBuffer("neighborCounter"));
  kh.setArg("SPH", 4, buffers.getBuffer("density"));
  kh.setArg("SPH", 5, buffers.getBuffer("pressure"));
  kh.setArg("SPH", 6, buffers.getBuffer("mass"));
  kh.setArg("SPH", 7, buffers.getBuffer("force"));
  kh.setArg("SPH", 8, smoothingLength);
  kh.setArg("SPH", 9, neighbor_amount);

  //__kernel void integration(__global float4* pos, __global float4* vel, __global float* density, __global float* mass, __global float4* force, float rho0, float dt)

  kh.setArg("integration", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("integration", 1, buffers.getBuffer("vel"));
  kh.setArg("integration", 2, buffers.getBuffer("density"));
  kh.setArg("integration", 3, buffers.getBuffer("mass"));
  kh.setArg("integration", 4, buffers.getBuffer("force"));
  kh.setArg("integration", 5, 0.59f);
  kh.setArg("integration", 6, dt);
  kh.setArg("integration", 7, xleft);

  /*__kernel void densityPressureCalc(__global float4* pos, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure,
    __global float* mass, float smoothingLength, float neighbor_amount, float rho0)*/

  kh.setArg("densityPressureCalc", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("densityPressureCalc", 1, buffers.getBuffer("neighbors"));
  kh.setArg("densityPressureCalc", 2, buffers.getBuffer("neighborCounter"));
  kh.setArg("densityPressureCalc", 3, buffers.getBuffer("density"));
  kh.setArg("densityPressureCalc", 4, buffers.getBuffer("pressure"));
  kh.setArg("densityPressureCalc", 5, buffers.getBuffer("mass"));
  kh.setArg("densityPressureCalc", 6, smoothingLength);
  kh.setArg("densityPressureCalc", 7, neighbor_amount);
  kh.setArg("densityPressureCalc", 8, 0.59f);

  //__kernel void neighbours(__global float4* pos, __global int* neighbour, __global int* counter, float smoothingLength, __global float* mass, __global float neighbor_amount)

  kh.setArg("neighbours", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("neighbours", 1, buffers.getBuffer("neighbors"));
  kh.setArg("neighbours", 2, buffers.getBuffer("neighborCounter"));
  kh.setArg("neighbours", 3, smoothingLength);
  kh.setArg("neighbours", 4, buffers.getBuffer("mass"));
  kh.setArg("neighbours", 5, neighbor_amount);

  kh.initKernels({ "updateGrid", "printGrid", "resetGrid" });
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
  int update_max = 200;
  int updates = 200;
  float xsign = 1.0f;
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
    int state = glfwGetKey(window, GLFW_KEY_R);
    if (state == GLFW_PRESS) {
      kh.reloadProgram();
      kh.setArg("SPH", 0, buffers.getBufferGL("pos_vbo"));
      kh.setArg("SPH", 1, buffers.getBuffer("vel"));
      kh.setArg("SPH", 2, buffers.getBuffer("neighbors"));
      kh.setArg("SPH", 3, buffers.getBuffer("neighborCounter"));
      kh.setArg("SPH", 4, buffers.getBuffer("density"));
      kh.setArg("SPH", 5, buffers.getBuffer("pressure"));
      kh.setArg("SPH", 6, buffers.getBuffer("mass"));
      kh.setArg("SPH", 7, buffers.getBuffer("force"));
      kh.setArg("SPH", 8, smoothingLength);
      kh.setArg("SPH", 9, neighbor_amount);

      //__kernel void integration(__global float4* pos, __global float4* vel, __global float* density, __global float* mass, __global float4* force, float rho0, float dt)

      kh.setArg("integration", 0, buffers.getBufferGL("pos_vbo"));
      kh.setArg("integration", 1, buffers.getBuffer("vel"));
      kh.setArg("integration", 2, buffers.getBuffer("density"));
      kh.setArg("integration", 3, buffers.getBuffer("mass"));
      kh.setArg("integration", 4, buffers.getBuffer("force"));
      kh.setArg("integration", 5, 0.59f);
      kh.setArg("integration", 6, dt);
      kh.setArg("integration", 7, xleft);

      /*__kernel void densityPressureCalc(__global float4* pos, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure,
      __global float* mass, float smoothingLength, float neighbor_amount, float rho0)*/

      kh.setArg("densityPressureCalc", 0, buffers.getBufferGL("pos_vbo"));
      kh.setArg("densityPressureCalc", 1, buffers.getBuffer("neighbors"));
      kh.setArg("densityPressureCalc", 2, buffers.getBuffer("neighborCounter"));
      kh.setArg("densityPressureCalc", 3, buffers.getBuffer("density"));
      kh.setArg("densityPressureCalc", 4, buffers.getBuffer("pressure"));
      kh.setArg("densityPressureCalc", 5, buffers.getBuffer("mass"));
      kh.setArg("densityPressureCalc", 6, smoothingLength);
      kh.setArg("densityPressureCalc", 7, neighbor_amount);
      kh.setArg("densityPressureCalc", 8, 0.59f);

      //__kernel void neighbours(__global float4* pos, __global int* neighbour, __global int* counter, float smoothingLength, __global float* mass, __global float neighbor_amount)

      kh.setArg("neighbours", 0, buffers.getBufferGL("pos_vbo"));
      kh.setArg("neighbours", 1, buffers.getBuffer("neighbors"));
      kh.setArg("neighbours", 2, buffers.getBuffer("neighborCounter"));
      kh.setArg("neighbours", 3, smoothingLength);
      kh.setArg("neighbours", 4, buffers.getBuffer("mass"));
      kh.setArg("neighbours", 5, neighbor_amount);
    }
    // Fixed timestep. Still lagging if rendering is slow. This is intended though
    while (accumulator > dt) {
      accumulator -= dt;
        //  queue.removeIndices(0, std::vector<int>(fluidIndices.begin() + test2, fluidIndices.begin() + test2 + 50));
      queue.acquireGL(&openglbuffers);
      queue.runRangeKernel(kh.get("resetGrid"), grid.getCellAmount());
      queue.runRangeKernel(kh.get("updateGrid"), particle_amount);
      queue.runRangeKernel(kh.get("printGrid"), particle_amount);
      queue.runRangeKernel(kh.get("neighbours"), particle_amount);
      queue.runRangeKernel(kh.get("densityPressureCalc"), particle_amount);
      queue.runRangeKernel(kh.get("SPH"), particle_amount);
      queue.runRangeKernel(kh.get("integration"), particle_amount);
      queue.releaseGL(&openglbuffers);
      queue.finish();
      updates++;
      xleft -= dt * xsign;
      kh.setArg("integration", 7, xleft);
    }
    if (updates >= 10000) {
      xsign *= -1.0f;
      updates = 0;
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