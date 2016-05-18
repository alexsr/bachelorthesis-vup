// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_TRACKBALLCAM_H
#define VUP_TRACKBALLCAM_H

#include "vup/defs.h"

namespace vup {

// Creates an OpenGL Shader Program from source and provides
// additional functionality to use and update the shader
// inside the renderloop.

class TrackballCam
{
public:
  TrackballCam(int width, int height, float sens, float r = 2.0, float zoomsens = 0.1f);
  ~TrackballCam();

  glm::mat4 getView();
  glm::mat4 getProjection();

  void update(GLFWwindow* window);

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
