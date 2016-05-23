#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/Rendering/ShaderProgram.h"
#include "vup/Rendering/TrackballCam.h"
#include "vup/Rendering/RenderData/SphereData.h"
#include "vup/Rendering/ParticleRenderer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

#define WIDTH 800
#define HEIGHT 600

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

int main()
{
  // GLFW
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Instanced Rendering", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwMakeContextCurrent(window);

  // GLEW
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
    return -1;
  }

  glViewport(0, 0, WIDTH, HEIGHT);

  glfwSetKeyCallback(window, key_callback);

  vup::TrackballCam cam(WIDTH, HEIGHT, 0.01f, 10.0f);

  vup::ShaderProgram simpleShader(SHADERS_PATH "/minimal.vert", SHADERS_PATH "/minimal.frag");
  simpleShader.updateUniform("proj", cam.getProjection());

  glm::vec3 vel[1000];
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < 1000; i++) {
    vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
  }

  std::vector<vup::particle::pos> translations(1000);
  srand(static_cast <unsigned> (time(0)));
  for (int i = 0; i < 1000; i++) {
    translations[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
    translations[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
  }
  
  float size = .1f;
  vup::SphereData sphere(size);
  vup::ParticleRenderer renderer(sphere, 1000);

  int test = 0;

  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (test > 100) {
      test = 0;
      for (int i = 0; i < 1000; i++) {
        vel[i].x = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].y = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
        vel[i].z = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
      }
    }
    for (int i = 0; i < 1000; i++) {
      translations[i].x += vel[i].x * 0.01f;
      translations[i].y += vel[i].y * 0.01f;
      translations[i].z += vel[i].z * 0.01f;
    }
    cam.update(window);
    simpleShader.updateUniform("view", cam.getView());
    simpleShader.use();
    renderer.updatePositions(&translations);
    renderer.execute(100);
    glfwPollEvents();
    glfwSwapBuffers(window);
    test++;
  }
  glfwTerminate();
  return 0;
}