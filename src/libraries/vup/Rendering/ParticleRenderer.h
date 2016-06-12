// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLERENDERER_H
#define VUP_PARTICLERENDERER_H

#include "vup/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"
#include "vup/particle.h"
#include "vup/ParticleHandling/VBO.h"
#include <map>

namespace vup {

// Renders particles with instanced rendering using the RenderData as the displayed object
// and the data in the instancedVBOs map to provide information for each instance.

class ParticleRenderer
{
public:
  // Creates an instance of a ParticleRenderer
  // * RenderData rd - vertices and normals of the reference object
  // * instancedVBOs - map containing VBO information utilized to create instanced vertex attrib data
  ParticleRenderer(vup::RenderData rd, std::map<std::string, vup::VBO> instancedVBOs);
  ~ParticleRenderer();
  // Renders n instances of RenderData
  void execute(int n);

private:
  int m_renderDataSize;
  RenderData m_renderData;
  GLuint m_vao;
  std::map<std::string, vup::VBO> m_instancedVBOs;

};

}

#endif
