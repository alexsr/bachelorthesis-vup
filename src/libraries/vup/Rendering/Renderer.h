// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_RENDERER_H
#define VUP_RENDERER_H

#include "vup/defs.h"
#include "vup/Rendering/RenderData/RenderData.h"
#include "vup/particle.h"

namespace vup {

// Renders the data passed in constructor
class Renderer
{
public:
  // Creates an instance of a Renderer
  // * RenderData rd - vertices and normals of the reference object
  Renderer(vup::RenderData rd);
  ~Renderer();
  // Renders n instances of RenderData
  void execute();

protected:
  int m_renderDataSize;
  RenderData m_renderData;
  GLuint m_vao;

};

}

#endif
