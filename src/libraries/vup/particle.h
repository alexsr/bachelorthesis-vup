// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLE_H
#define VUP_PARTICLE_H

#include "vup/defs.h"

namespace vup {

typedef std::vector<glm::vec4> position;
typedef std::vector<glm::vec4> velocity;
typedef std::vector<glm::vec4> color;
typedef std::vector<int> type;

struct particle {
  cl_int id;
  cl_int type;
  cl_float mass;
  cl_float density;
  cl_float viscosity;
};

template <int C>
struct connections {
  int id;
  struct {
    int to;
    float min_length;
    float max_length;
    float spring_constant;
  } connection[C];
};

}

#endif
