// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SPHEREDATA_H
#define VUP_SPHEREDATA_H

#include "vup/Core/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"

namespace vup {

// Creates the vertices and normals of a sphere.
// Radius, horizontal and vertical resolution can be specified.
class SphereData : public RenderData
{
public:
  // Creates an instance of sphere data
  // * float r - radius of the sphere
  // * int hres - horizontal resolution
  // * int vres - vertical resolution
  SphereData(float r = 1.0, int hres = 20, int vres = 20);
  virtual ~SphereData();

private:
  // Creates a point on the sphere using the sphere formula.
  glm::vec4 createPoint(float theta, float phi);
  float m_radius;
  int m_hres;
  int m_vres;

};

}

#endif
