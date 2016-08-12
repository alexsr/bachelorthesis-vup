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
    translations[i].y = -1.0f;// -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + 1.0f * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].w = 1.0f;
  }
  srand(static_cast <unsigned> (time(0)));
  vup::color color(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    color[i].r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    color[i].a = 0.3f;
  }
  vup::velocity vel(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    vel[i] = glm::vec4(0.0f);
  }
  vup::type type(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    type[i] = static_cast<int>(-1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f)));
  }
  int neighbor_amount = 100;
  std::vector<int> neighbors(particle_amount * neighbor_amount);
  for (int i = 0; i < particle_amount * neighbor_amount; i++) {
    neighbors[i] = 0;
  }
  std::vector<int> neighborCounter(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    neighborCounter[i] = 0;
  }
  std::vector<vup::particle> particles(particle_amount);
  /*for (int i = 0; i < particle_amount; i++) {
    particles[i].id = 0;
    particles[i].type = 0;
    particles[i].mass = 0.00005f;
    particles[i].density = 0.59f;
    particles[i].viscosity = 0.890f;
    particles[i].lambda = 0.0f;
    particles[i].pressure = 0.0f;
    particles[i].vis = { 0.0f, 0.0f, 0.0f, 0.0f };
    particles[i].force = { 0.0f, 0.0f, 0.0f, 0.0f };
  }*/
  std::vector<float> densities(particle_amount);
  std::vector<glm::vec4> forces(particle_amount);
  std::vector<float> pressure(particle_amount);
  for (int i = 0; i < particle_amount; i++) {
    densities[i] = 1000.0f;
    forces[i] = glm::vec4(0.0f);
    pressure[i] = 0.0f;
  }

  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::BufferHandler buffers(clBasis.context());
  buffers.createVBOData("pos", 2, particle_amount, 4, translations, true, GL_STREAM_DRAW);
  buffers.createVBOData("color", 3, particle_amount, 4, color, true, GL_STATIC_DRAW);

  float size = .05f;
  vup::SphereData sphere(size, 12, 12);
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
  vup::UniformGrid grid(500, 0.5f, 1.0f, clBasis.context(), CL_MEM_READ_WRITE);
  //queue.writeBuffer(grid.getGridBuffer(), sizeof(int) * grid.getMaxGridCapacity(), &grid.getGridData()[0]);
  //queue.writeBuffer(grid.getCounterBuffer(), sizeof(int) * grid.getCellAmount(), &grid.getCounterData()[0]);
  buffers.createBufferGL("pos_vbo", CL_MEM_READ_WRITE, "pos");
  buffers.createBuffer<vup::velocity>("vel", CL_MEM_READ_WRITE, vel.size());
  queue.writeBuffer(buffers.getBuffer("vel"), sizeof(glm::vec4) * vel.size(), &vel[0]);
  buffers.createBuffer<float>("pressure", CL_MEM_READ_WRITE, pressure.size());
  queue.writeBuffer(buffers.getBuffer("pressure"), sizeof(float) * pressure.size(), &pressure[0]);
  buffers.createBuffer<float>("density", CL_MEM_READ_WRITE, densities.size());
  queue.writeBuffer(buffers.getBuffer("density"), sizeof(float) * densities.size(), &densities[0]);
  buffers.createBuffer<glm::vec4>("force", CL_MEM_READ_WRITE, forces.size());
  queue.writeBuffer(buffers.getBuffer("force"), sizeof(glm::vec4) * forces.size(), &forces[0]);
  buffers.createBuffer<vup::position>("nextpos", CL_MEM_READ_WRITE, translations.size());
  queue.writeBuffer(buffers.getBuffer("nextpos"), sizeof(glm::vec4) * translations.size(), &translations[0]);/*
  buffers.createBuffer<vup::particle>("particles", CL_MEM_READ_WRITE, particles.size());
  queue.writeBuffer(buffers.getBuffer("particles"), sizeof(vup::particle) * particles.size(), &particles[0]);*/
  buffers.createBuffer<int>("neighbors", CL_MEM_READ_WRITE, neighbors.size());
  queue.writeBuffer(buffers.getBuffer("neighbors"), sizeof(int) * neighbors.size(), &neighbors[0]);
  buffers.createBuffer<int>("neighborCounter", CL_MEM_READ_WRITE, neighborCounter.size());
  queue.writeBuffer(buffers.getBuffer("neighborCounter"), sizeof(int) * neighborCounter.size(), &neighborCounter[0]);
 // queue.setTypeIndices(VUP_FLUID, CL_MEM_READ_WRITE, fluidIndices, particle_amount);
  //queue.setTypeIndices(VUP_RIGID, CL_MEM_READ_WRITE, rigidIndices, particle_amount);
  
  float dt = 0.001f;
  float camdt = 0.01f;
  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();
  vup::KernelHandler kh(clBasis.context(), clBasis.device(), OPENCL_KERNEL_PATH "/sph_force.cl", {"updateDensity", "findNeighbors", "integrate", "updateGrid", "resetGrid", "printGrid" });
  
  float smoothingLength = .5f;
 // kh.setArg("move", 3, queue.getIndexBuffer(VUP_FLUID));
  //kh.initKernel("test");
 // __kernel void move(__global float4* pos, __global float4* next, __global float4* vel, __global particle* particles, __global int* grid, __global int* counter, float cellSize, int lineSize, int cellCapacity, float dt) {
  
  kh.setArg("findNeighbors", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("findNeighbors", 1, buffers.getBuffer("neighborCounter"));
  kh.setArg("findNeighbors", 2, buffers.getBuffer("neighbors"));
  kh.setArg("findNeighbors", 3, neighbor_amount);
  kh.setArg("findNeighbors", 4, smoothingLength);
  kh.setArg("findNeighbors", 5, grid.getCounterBuffer());
  kh.setArg("findNeighbors", 6, grid.getGridBuffer());
  kh.setArg("findNeighbors", 7, grid.getCellSize());
  kh.setArg("findNeighbors", 8, grid.getLineSize());
  kh.setArg("findNeighbors", 9, grid.getCellCapacity());

  //__kernel void updateDensity(__global float4* next, __global particle* particles, __global int* counter, __global int* neighbors, int neighborhoodLength, float smoothingLength) {

  kh.setArg("updateDensity", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("updateDensity", 1, buffers.getBuffer("density"));
  kh.setArg("updateDensity", 2, buffers.getBuffer("pressure"));
  kh.setArg("updateDensity", 3, buffers.getBuffer("neighborCounter"));
  kh.setArg("updateDensity", 4, buffers.getBuffer("neighbors"));
  kh.setArg("updateDensity", 5, neighbor_amount);
  kh.setArg("updateDensity", 6, smoothingLength);

  kh.initKernel("calcForces");
  kh.setArg("calcForces", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("calcForces", 1, buffers.getBuffer("vel"));
  kh.setArg("calcForces", 2, buffers.getBuffer("density"));
  kh.setArg("calcForces", 3, buffers.getBuffer("pressure"));
  kh.setArg("calcForces", 4, buffers.getBuffer("force"));
  kh.setArg("calcForces", 5, buffers.getBuffer("neighbors"));
  kh.setArg("calcForces", 6, buffers.getBuffer("neighborCounter"));
  kh.setArg("calcForces", 7, smoothingLength);
  kh.setArg("calcForces", 8, neighbor_amount);


  kh.setArg("integrate", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("integrate", 1, buffers.getBuffer("nextpos"));
  kh.setArg("integrate", 2, buffers.getBuffer("vel"));
  kh.setArg("integrate", 3, buffers.getBuffer("density"));
  kh.setArg("integrate", 4, buffers.getBuffer("force"));
  kh.setArg("integrate", 5, dt);
 
  
  kh.setArg("updateGrid", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("updateGrid", 1, grid.getGridBuffer());
  kh.setArg("updateGrid", 2, grid.getCounterBuffer());
  kh.setArg("updateGrid", 3, grid.getCellSize());
  kh.setArg("updateGrid", 4, grid.getLineSize());
  kh.setArg("updateGrid", 5, grid.getCellCapacity());

  kh.setArg("printGrid", 0, buffers.getBufferGL("pos_vbo"));
  kh.setArg("printGrid", 1, grid.getGridBuffer());
  kh.setArg("printGrid", 2, grid.getCounterBuffer());
  kh.setArg("printGrid", 3, grid.getCellSize());
  kh.setArg("printGrid", 4, grid.getLineSize());
  kh.setArg("printGrid", 5, grid.getCellCapacity());

  kh.setArg("resetGrid", 0, grid.getCounterBuffer());
  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  double accumulator = 0.0;
  int frames = 0;
  int update_max = 0;
  int updates = update_max;
  // Main loop
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window)) {
    vup::clearGL();
    accumulator += glfwGetTime() - currentTime;
    currentTime = glfwGetTime();
    frames++;
    lastTime = vup::updateFramerate(currentTime, lastTime, window, frames);
    // Fixed timestep. Still lagging if rendering is slow. This is intended though
    glFinish();
    while (accumulator > dt) {
      accumulator -= dt;
        //  queue.removeIndices(0, std::vector<int>(fluidIndices.begin() + test2, fluidIndices.begin() + test2 + 50));
      queue.acquireGL(&openglbuffers);
      queue.finish();
      //queue.runRangeKernel(kh.get("collide"), particle_amount);
      //queue.runRangeKernel(kh.get("predict"), particle_amount);
      if (updates >= update_max) {
        updates = 0;
        //queue.runRangeKernel(kh.get("resetGrid"), grid.getCellAmount());
        //queue.runRangeKernel(kh.get("updateGrid"), particle_amount);
        //queue.runRangeKernel(kh.get("printGrid"), particle_amount);
      }
      queue.runRangeKernel(kh.get("findNeighbors"), particle_amount);
      queue.finish();
      queue.runRangeKernel(kh.get("updateDensity"), particle_amount);
      queue.finish();
      //queue.runRangeKernel(kh.get("calcLambda"), particle_amount);
      //queue.runRangeKernel(kh.get("calcPosDelta"), particle_amount);
      //queue.runRangeKernel(kh.get("fakecollision"), particle_amount);
      queue.runRangeKernel(kh.get("calcForces"), particle_amount);
      queue.finish();
      queue.runRangeKernel(kh.get("integrate"), particle_amount);
      queue.finish();
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