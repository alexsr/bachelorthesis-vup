#include "ParticleRenderer.h"

vup::ParticleRenderer::ParticleRenderer(RenderData rd, int size)
{
  m_size = size;
  m_rd = rd;
  m_rdsize = rd.getSize();

  // Store instance data in an array buffer
  glGenBuffers(1, &m_posVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m_size, NULL, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
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
  glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glVertexAttribDivisor(1, 1); // Tell OpenGL this is an instanced vertex attribute.
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

void vup::ParticleRenderer::updatePositions(std::vector<vup::particle::pos>* data)
{
  glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
  vup::particle::pos * vertexArray = (vup::particle::pos *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (int i = 0; i < 1000; i++) {
    vertexArray[i] = data->at(i);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
