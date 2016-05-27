// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SPHEREDATA_H
#define VUP_SPHEREDATA_H

#include "vup/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"

namespace vup {

// Creates the vertices and normals of a sphere.
// Radius, horizontal and vertical resolution can be specified.

class SphereData : public RenderData
{
public:
  SphereData(float r = 1.0, int hres = 30, int vres = 30);
  virtual ~SphereData();
  // Creates a point on the sphere using the sphere formula.
  glm::vec4 createPoint(float theta, float phi);
  virtual int getResolution() { return m_hres * m_vres; }

private:
  float m_r;
  int m_hres;
  int m_vres;

};

}

#endif
