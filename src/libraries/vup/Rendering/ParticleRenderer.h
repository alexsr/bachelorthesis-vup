// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLERENDERER_H
#define VUP_PARTICLERENDERER_H

#include "vup/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"
#include "vup/particle.h"

namespace vup {

class ParticleRenderer
{
public:
  ParticleRenderer(RenderData rd, int size, GLuint posVBO, std::vector<std::pair<GLuint, int>> instancedVBOS);
  ~ParticleRenderer();
  void execute(int amount);

private:
  int m_size;
  int m_rdsize;
  RenderData m_rd;
  GLuint m_vao;

};

}

#endif
