// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#include "vup/Core/defs.h"
#include "vup/Core/utils.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"
#include "vup/Core/ParticleSimulation.h"

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
  vup::ShaderProgram simpleShader(SHADERS_PATH "/instancedPhong.vert", SHADERS_PATH "/instancedPhong.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  vup::ParticleSimulation ps(RESOURCES_PATH "/config.txt", 0, CL_DEVICE_TYPE_GPU, 0);
  
  vup::SphereData* sphere = new vup::SphereData(ps.getSize(), 10, 10);
  vup::ParticleRenderer* renderer = new vup::ParticleRenderer(*sphere, ps.getInteropVBOs());
  double dt = 0.01f;
  double renderUpdate = 1.0 / 60.0;
  glfwSetTime(0.0);
  double currentTime = glfwGetTime();
  double lastTime = glfwGetTime();
  double frameTime = lastTime;
  double accumulator = 0.0;
  double renderAccumulator = 0.0;
  double frames = 0.0;
  // Main loop
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  while (!glfwWindowShouldClose(window)) {
    vup::clearGL();
    currentTime = glfwGetTime();
    accumulator += currentTime - lastTime;
    if (currentTime - frameTime > 1) {
      vup::updateFramerate(currentTime, frameTime, frames, window);
      frameTime = currentTime;
      frames = 0;
    }
    frames += 1;
    lastTime = currentTime;
    accumulator = ps.runAccumulated(accumulator, dt);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
      ps.reload();
      ps.init();
      sphere = new vup::SphereData(ps.getSize(), 10, 10);
      renderer = new vup::ParticleRenderer(*sphere, ps.getInteropVBOs());
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
      ps.reloadKernel();
    }
    cam.update(window, dt);
    simpleShader.updateUniform("view", cam.getView());
    simpleShader.use();

    renderer->execute(ps.getParticleCount());
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}