// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLE_H
#define VUP_PARTICLE_H

#include "vup/defs.h"

namespace vup {

typedef struct {
  cl_float mass;
  cl_float density;
  cl_float viscosity;
  cl_float pressure;
  cl_float4 force;
} particle;

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
