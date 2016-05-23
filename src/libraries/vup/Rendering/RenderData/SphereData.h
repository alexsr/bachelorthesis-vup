// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SPHEREDATA_H
#define VUP_SPHEREDATA_H

#include "vup/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"

namespace vup {

class SphereData : public RenderData
{
public:
  SphereData(float r = 1.0, int hres = 30, int vres = 30);
  virtual ~SphereData();

  glm::vec4 createPoint(float theta, float phi);

private:
  float m_r;
  int m_hres;
  int m_vres;

};

}

#endif