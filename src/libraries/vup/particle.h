// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLE_H
#define VUP_PARTICLE_H

#include "vup/defs.h"

namespace vup {

struct particle {
  glm::vec4 pos;
  glm::vec4 vel;
  int type;
  float mass;
  float density;
  float viscosity;
  glm::vec4 color;
};

}

#endif
