// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_DEFS_H
#define VUP_DEFS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <map>

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/OpenGL.h>
#else
#include "CL/cl.hpp"
#include "CL/cl_gl.h"
#endif

namespace vup {

typedef std::vector<float> floatdata;
typedef std::vector<glm::vec4> vec4data;

enum datatype { EMPTY, INT, FLOAT, VEC4 };

typedef std::pair<datatype, std::string> datavalue;

typedef std::map<std::string, datavalue> datamap;
typedef std::vector<std::string> identifiers;
typedef std::map<std::string, datatype> typeIdentifiers;


}

#endif
