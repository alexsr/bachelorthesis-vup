#include "ParticleRenderer.h"

vup::ParticleRenderer::ParticleRenderer(RenderData rd, int size, GLuint posVBO, std::vector<std::pair<GLuint, int>> instancedVBOS)
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
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, posVBO);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glVertexAttribDivisor(1, 1); // Tell OpenGL this is an instanced vertex attribute.
  for (int i = 0; i < instancedVBOS.size(); i++) {
    glEnableVertexAttribArray(i + 2);
    glBindBuffer(GL_ARRAY_BUFFER, instancedVBOS[i].first);
    glVertexAttribPointer(i + 2, instancedVBOS[i].second, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(i + 2, 1); // Tell OpenGL this is an instanced vertex attribute.
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
