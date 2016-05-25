#ifndef VUP_PARTICLE_H
#define VUP_PARTICLE_H

#include "vup/defs.h"

namespace vup {

struct particle {
  typedef glm::vec4 pos;
  typedef glm::vec4 vel;
  typedef glm::vec4 color;
  typedef int type;
};

}

#endif
