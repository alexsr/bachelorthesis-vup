#ifndef VUP_PARTICLE_H
#define VUP_PARTICLE_H

#include "vup/defs.h"

namespace vup {

struct particle {
  struct pos {
    float x, y, z;
  };
  struct vel {
    float x, y, z;
  };
};

}

#endif
