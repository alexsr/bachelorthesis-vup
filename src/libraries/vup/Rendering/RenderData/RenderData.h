// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_RENDERDATA_H
#define VUP_RENDERDATA_H

#include "vup/defs.h"
#include <vector>

namespace vup {

// Contains vertices and normals of render objects.
// This class is not meant to be used itself but
// serves as an abstract class to all render data classes.
// All the shared methods are specified here.

class RenderData
{
public:
  RenderData();
  virtual ~RenderData();
  std::vector<glm::vec4> getVertices() { return m_vertices; }
  std::vector<glm::vec3> getNormals() { return m_normals; }
  // Vertices are glm::vec4
  int sizeOfVertices() { return sizeof(glm::vec4) * m_vertices.size(); }
  // Normals are glm::vec3
  int sizeOfNormals() { return sizeof(glm::vec3) * m_normals.size(); }
  int getSize() { return m_size; }

protected:
  std::vector<glm::vec4> m_vertices;
  std::vector<glm::vec3> m_normals;
  int m_size;
};

}

#endif
