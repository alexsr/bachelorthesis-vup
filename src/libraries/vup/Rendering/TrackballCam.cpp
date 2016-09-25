#include "TrackballCam.h"

vup::TrackballCam::TrackballCam(int width, int height, float sens, float r, float zoomsens, glm::vec3 center, float fov, float near, float far) {
  m_sens = sens;
  m_zoomsens = zoomsens;
  m_width = width;
  m_height = height;
  m_center = center;

  m_width = width;
  m_height = height;
  m_oldX = 0.0;
  m_oldY = 0.0;
  m_x = 0.0;
  m_y = 0.0;
 
  m_theta = glm::pi<double>() / 2.0;
  m_phi = 0.0;
  m_radius = r;

  m_cameraPos.x = m_center.x + m_radius * glm::sin(m_theta) * glm::sin(m_phi);
  m_cameraPos.y = m_center.y + m_radius * glm::cos(m_theta);
  m_cameraPos.z = m_center.z + m_radius * glm::sin(m_theta) * glm::cos(m_phi);
  
  m_view = glm::lookAt(m_cameraPos, m_center, glm::vec3(0.0, 1.0f, 0.0));
  m_projection = glm::perspective(glm::radians(fov), m_width / (float)m_height, near, far);
}

vup::TrackballCam::~TrackballCam()
{
}

glm::mat4 vup::TrackballCam::getView()
{
  return m_view;
}

glm::mat4 vup::TrackballCam::getProjection()
{
  return m_projection;
}

void vup::TrackballCam::update(GLFWwindow * window, float dt)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    if (!m_lmb_pressed) {
      m_lmb_pressed = true;
      m_oldX = x;
      m_oldY = y;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    double changeX = (x - m_x) * m_sens * dt;
    double changeY = (y - m_y) * m_sens * dt;

    m_theta -= changeY;
    if (m_theta < 0.01) {
      m_theta = 0.01;
    } else if (m_theta > glm::pi<double>() - 0.01) {
      m_theta = glm::pi<double>() - 0.01;
    }

    m_phi -= changeX;
    if (m_phi < 0) {
      m_phi += 2 * glm::pi<double>();
    } else if (m_phi > 2 * glm::pi<double>()) {
      m_phi -= 2 * glm::pi<double>();
    }
    glfwSetCursorPos(window, m_oldX, m_oldY);
  }
  else {
    m_lmb_pressed = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_x = x;
    m_y = y;
  }
  
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    m_radius -= m_zoomsens * dt;
  }
  else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    m_radius += m_zoomsens * dt;
  }
  if (m_radius < 0.1) {
    m_radius = 0.1;
  }

  m_cameraPos.x = m_center.x + m_radius * glm::sin(m_theta) * glm::sin(m_phi);
  m_cameraPos.y = m_center.y + m_radius * glm::cos(m_theta);
  m_cameraPos.z = m_center.z + m_radius * glm::sin(m_theta) * glm::cos(m_phi);

  m_view = glm::lookAt(m_cameraPos, m_center, glm::vec3(0.0f, 1.0f, 0.0f));

}
