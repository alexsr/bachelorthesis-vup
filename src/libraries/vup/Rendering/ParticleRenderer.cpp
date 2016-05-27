#include "ParticleRenderer.h"

vup::ParticleRenderer::ParticleRenderer(RenderData rd, int size, std::map<std::string, vup::VBO> instancedVBOS)
{
  m_size = size;
  m_rd = rd;
  m_rdsize = rd.getSize();

  GLuint renderVBO;
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &renderVBO);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderVBO);
  glBufferData(GL_ARRAY_BUFFER, m_rd.sizeOfVertices(), &(m_rd.getVertices())[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  // Also set instance data
  std::map<std::string, vup::VBO>::iterator it;
  for (it = instancedVBOS.begin(); it != instancedVBOS.end(); it++) {
    int loc = it->second.location;
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, it->second.handle);
    glVertexAttribPointer(loc, it->second.size, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(loc, 1); // Tell OpenGL this is an instanced vertex attribute.
  }
  glBindVertexArray(0);
}

vup::ParticleRenderer::~ParticleRenderer()
{
  
}

void vup::ParticleRenderer::execute(int amount)
{
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArraysInstanced(GL_TRIANGLES, 0, m_rdsize, amount);
  glBindVertexArray(0);
}
