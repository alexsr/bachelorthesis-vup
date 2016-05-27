#ifndef VUP_VBO_H
#define VUP_VBO_H

#include "vup/defs.h"

namespace vup {

class VBO {
public:
  VBO();
  VBO(GLuint h, int loc, int s);
  ~VBO();
  GLuint handle;
  int size;
  int location;
};

}

#endif
