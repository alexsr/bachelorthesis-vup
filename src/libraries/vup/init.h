// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_INIT_H
#define VUP_INIT_H

#include "vup/defs.h"
#include <iostream>

namespace vup {

GLFWwindow* createWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
  GLint glfwError = glfwInit();
  GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, share);
  if (window == nullptr) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    exit(-1);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);
  return window;
}

void initGLEW() {
  GLenum glewError = glewInit();
  if (GLEW_OK != glewError) {
    std::cout << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
    exit(-1);
  }
}

}

#endif
