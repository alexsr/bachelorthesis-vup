// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_VBO_H
#define VUP_VBO_H

#include "vup/defs.h"

namespace vup {

// Represent a vbo on the cpu side. This holds all the important
// information for accessing and using an vbo.
// All parameters are public to ease usablility.
// This could also be a struct but is not to allow easy extension.
class VBO {
public:
  VBO();
  VBO(GLuint h, int loc, int f);
  ~VBO();
  GLuint handle;
  int format;
  int location;
};

}

#endif
