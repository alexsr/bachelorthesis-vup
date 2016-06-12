#include "ParticleRenderer.h"

vup::ParticleRenderer::ParticleRenderer(RenderData rd, std::map<std::string, vup::VBO> instancedVBOs) : vup::Renderer(rd)
{
  
  m_instancedVBOs = instancedVBOs;

  glBindVertexArray(m_vao);
  std::map<std::string, vup::VBO>::iterator it;
  // Use iterator to loop through map. Loc is specified in VBO to allow this behavior,
  // because without it there would be no way to ensure the right order of vbos.
  for (it = m_instancedVBOs.begin(); it != m_instancedVBOs.end(); it++) {
    // it->second is the VBO in the map.
    // The VBOs BufferData is already set in the buffer handler.
    int loc = it->second.location;
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, it->second.handle);
    glVertexAttribPointer(loc, it->second.size, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(loc, 1); // Make vertex attribute at loc instanced
  }
  glBindVertexArray(0);
}

vup::ParticleRenderer::~ParticleRenderer()
{
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &m_vao);
}

void vup::ParticleRenderer::execute(int n)
{
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArraysInstanced(GL_TRIANGLES, 0, m_renderDataSize, n);
  glBindVertexArray(0);
}
