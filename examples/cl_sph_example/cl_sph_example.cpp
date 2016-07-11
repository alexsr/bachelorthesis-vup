#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/utils.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/ParticleHandling/DataLoader.h"
#include "vup/ParticleHandling/BufferHandler.h"
#include "vup/ParticleHandling/UniformGrid.h"
#include "vup/OpenCLUtil/OpenCLUtil.h"

#include <string>
#include <ctime>

#define WIDTH 1920
#define HEIGHT 1080

int main()
{
  GLFWwindow* window = vup::createWindow(WIDTH, HEIGHT, "Instanced Rendering", nullptr, nullptr);
  glfwSetKeyCallback(window, vup::closeWindowCallback);

  vup::initGLEW();
  glViewport(0, 0, WIDTH, HEIGHT);

  vup::TrackballCam cam(WIDTH, HEIGHT, 1.0f, 10.0f, 10.0f, glm::vec3(0.0f, 0.0f, 0.0f));
  vup::ShaderProgram simpleShader(SHADERS_PATH "/instanced.vert", SHADERS_PATH "/instanced.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  vup::OpenCLBasis clBasis(1, CL_DEVICE_TYPE_GPU, 0);
  vup::BufferHandler buffers(clBasis.context());
  vup::DataLoader pdl(RESOURCES_PATH "/data/test.vpd");

  int particle_amount = pdl.particleAmount();
  std::vector<glm::vec4> translations = pdl.getVec4Dataset("pos");
  std::cout << translations.size() << ";" << std::endl;
  std::vector<glm::vec4> color = pdl.getVec4Dataset("color");
  std::cout << color.size() << ";" << std::endl;
  for (int i = 0; i < color.size(); i++) {
    color[i] = (color.at(i) + glm::vec4(1.0)) / 2.0f;
  }
  std::vector<glm::vec4> vel = pdl.getVec4Dataset("vel");
  std::cout << vel.size() << ";" << std::endl;
  std::vector<float> mass = pdl.getFloatDataset("mass");
  std::cout << mass.size() << ";" << std::endl;
  std::vector<int> type(particle_amount);
  std::vector<float> density = pdl.getFloatDataset("density");
  std::vector<float> pressure = pdl.getFloatDataset("pressure");
  std::vector<glm::vec4> force = pdl.getVec4Dataset("force");

  int neighbor_amount = 20;
  std::vector<int> neighborCounter(particle_amount);
  for (int i = 0; i < neighborCounter.size(); i++) {
    neighborCounter[i] = 0;
  }
  std::vector<int> neighbors(particle_amount * neighbor_amount);
  for (int i = 0; i < neighbors.size(); i++) {
    neighbors[i] = 0;
  }

  vup::SphereData sphere(pdl.getFloatConst("size"), 12, 12);
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
  vup::UniformGrid grid(20, 10, 2.0f, clBasis.context(), CL_MEM_READ_WRITE);
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
  buffers.createBuffer<int>("neighbors", CL_MEM_READ_WRITE, neighbors.size());
  queue.writeBuffer(buffers.getBuffer("neighbors"), sizeof(int) * neighbors.size(), &neighbors[0]);
  buffers.createBuffer<int>("neighborCounter", CL_MEM_READ_WRITE, neighborCounter.size());
  queue.writeBuffer(buffers.getBuffer("neighborCounter"), sizeof(int) * neighborCounter.size(), &neighborCounter[0]);

  float dt = 0.0007f;
  float camdt = 0.01f;
  std::vector<cl::Memory> openglbuffers = buffers.getGLBuffers();
  vup::KernelHandler kh(clBasis.context(), clBasis.device(), OPENCL_KERNEL_PATH "/sph_force.cl", {"integrate", "calcForces", "calcPressure", "findNeighbors" });

  float smoothingLength = pdl.getFloatConst("smoothingLength");
  float restDensity = pdl.getFloatConst("restDensity");

  kh.initKernels({ "updateGrid", "printGrid", "resetGrid" });
  std::vector<std::string> posKernelNames = { "calcPressure", "calcForces", "findNeighbors", "integrate", "updateGrid", "printGrid" };
  std::vector<std::string> onGridNames = { "findNeighbors", "updateGrid", "printGrid" };
  std::vector<std::string> calcKernelNames = { "calcPressure", "calcForces" };

  kh.setArg(posKernelNames, 0, buffers.getBufferGL("pos_vbo"));

  kh.setArg(onGridNames, 1, grid.getGridBuffer());
  kh.setArg(onGridNames, 2, grid.getCounterBuffer());
  kh.setArg(onGridNames, 3, grid.getGridRadius());
  kh.setArg(onGridNames, 4, grid.getCellsPerLine());
  kh.setArg(onGridNames, 5, grid.getCellCapacity());

  kh.setArg("findNeighbors", 6, buffers.getBuffer("neighbors"));
  kh.setArg("findNeighbors", 7, buffers.getBuffer("neighborCounter"));
  kh.setArg("findNeighbors", 8, neighbor_amount);
  kh.setArg("findNeighbors", 9, smoothingLength);

  kh.setArg(calcKernelNames, 1, buffers.getBuffer("neighbors"));
  kh.setArg(calcKernelNames, 2, buffers.getBuffer("neighborCounter"));
  kh.setArg(calcKernelNames, 3, buffers.getBuffer("density"));
  kh.setArg(calcKernelNames, 4, buffers.getBuffer("pressure"));
  kh.setArg(calcKernelNames, 5, buffers.getBuffer("mass"));
  kh.setArg(calcKernelNames, 6, smoothingLength);
  kh.setArg(calcKernelNames, 7, neighbor_amount);

  kh.setArg("calcPressure", 8, restDensity);

  kh.setArg("calcForces", 8, buffers.getBuffer("vel"));
  kh.setArg("calcForces", 9, buffers.getBuffer("force"));

  kh.setArg("integrate", 1, buffers.getBuffer("vel"));
  kh.setArg("integrate", 2, buffers.getBuffer("density"));
  kh.setArg("integrate", 3, buffers.getBuffer("mass"));
  kh.setArg("integrate", 4, buffers.getBuffer("force"));
  kh.setArg("integrate", 5, restDensity);
  kh.setArg("integrate", 6, dt);

  kh.setArg("printGrid", 6, buffers.getBufferGL("color"));

  kh.setArg("resetGrid", 0, grid.getCounterBuffer());

  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  double accumulator = 0.0;
  int frames = 0;
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
    int gridUpdate = 0;
    while (accumulator > dt) {
      accumulator -= dt;
      queue.acquireGL(&openglbuffers);
      if (gridUpdate > 0) {
        gridUpdate = 0;
        queue.runRangeKernel(kh.get("resetGrid"), grid.getCellAmount());
        queue.runRangeKernel(kh.get("updateGrid"), particle_amount);
        //queue.runRangeKernel(kh.get("printGrid"), particle_amount);
      }
      queue.runRangeKernel(kh.get("findNeighbors"), particle_amount);
      queue.runRangeKernel(kh.get("calcPressure"), particle_amount);
      queue.runRangeKernel(kh.get("calcForces"), particle_amount);
      //queue.runRangeKernel(kh.get("integrate"), particle_amount);
      queue.releaseGL(&openglbuffers);
      gridUpdate++;
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