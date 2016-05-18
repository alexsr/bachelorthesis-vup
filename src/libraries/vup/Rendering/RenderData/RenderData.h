// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_RENDERDATA_H
#define VUP_RENDERDATA_H

#include "vup/defs.h"
#include <vector>

namespace vup {

// Creates an OpenGL Shader Program from source and provides
// additional functionality to use and update the shader
// inside the renderloop.

class RenderData
{
public:
  RenderData();
  virtual ~RenderData() = 0;
  // Activates the use of the shader program.
  std::vector<glm::vec3> getVertices() { return m_vertices; }
  std::vector<glm::vec3> getNormals() { return m_normals; }
  int sizeOfVertices() { return sizeof(glm::vec3) * m_vertices.size(); }
  int sizeOfNormals() { return sizeof(glm::vec3) * m_normals.size(); }

protected:
  std::vector<glm::vec3> m_vertices;
  std::vector<glm::vec3> m_normals;
};

}

#endif
