// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLERENDERER_H
#define VUP_PARTICLERENDERER_H

#include "vup/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"

struct pos {
  float x, y, z;
};

namespace vup {

// Creates an OpenGL Shader Program from source and provides
// additional functionality to use and update the shader
// inside the renderloop.

class ParticleRenderer
{
public:
  ParticleRenderer(RenderData rd, int size);
  ~ParticleRenderer();
  void execute(int amount);
  void updatePositions(std::vector<pos>* positions);

private:
  int m_size;
  int m_rdsize;
  RenderData m_rd;
  GLuint m_vao;
  GLuint m_posVBO;

};

}

#endif
