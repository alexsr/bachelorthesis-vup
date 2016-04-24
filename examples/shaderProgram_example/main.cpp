#include "vup/defs.h"
#include "vup/Rendering/ShaderProgram.h"

#include <iostream>
#include <fstream>
#include <string>

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
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGl", nullptr, nullptr);
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

  vup::ShaderProgram simpleShader(SHADERS_PATH "/minimal.vert", SHADERS_PATH "/minimal.frag");
  
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}