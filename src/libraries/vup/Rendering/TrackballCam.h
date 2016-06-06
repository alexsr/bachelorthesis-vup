// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_TRACKBALLCAM_H
#define VUP_TRACKBALLCAM_H

#include "vup/defs.h"

namespace vup {

// Provides the functionality of a trackball camera.
// The view matrix for the current camera view is calculated and
// is accessable as well as the projection.
// A lot of parameters of this camera can be specified.

class TrackballCam
{
public:
  // The given options proved to be reasonable.
  TrackballCam(int width, int height, float sens = 0.01f, float r = 2.0, float zoomsens = 1.0f, float fov = 60.0f, float near = 0.001f, float far = 1000.0f);
  ~TrackballCam();

  glm::mat4 getView();
  glm::mat4 getProjection();

  // Updates the camera view using mouse controls, which is the reason
  // why the window has to be passed.
  void update(GLFWwindow* window, float dt);

private:
  
  glm::vec3 m_center;
  glm::vec3 m_cameraPos;
  glm::mat4 m_view;
  glm::mat4 m_projection;

  float m_sens;
  float m_zoomsens;
  float m_radius;
  float m_theta;
  float m_phi;
  float m_x;
  float m_y;
  float m_oldX;
  float m_oldY;
  int m_width;
  int m_height;
  bool m_lmb_pressed;

};


}

#endif
