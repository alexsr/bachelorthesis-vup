#include "ParticleRenderer.h"

vup::ParticleRenderer::ParticleRenderer(RenderData rd, int size, std::map<std::string, vup::VBO> instancedVBOS)
{
  m_size = size;
  m_rd = rd;
  m_rdsize = rd.getSize();

  m_instancedVBOS = instancedVBOS;

  // Create vbo for rendering using specified RenderData.
  GLuint renderVBO;
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &renderVBO);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderVBO);
  glBufferData(GL_ARRAY_BUFFER, m_rd.sizeOfVertices(), &(m_rd.getVertices())[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

  std::map<std::string, vup::VBO>::iterator it;
  // Use iterator to loop through map. Loc is specified in VBO to allow this behavior,
  // because without it there would be no way to ensure the right order of vbos.
  for (it = m_instancedVBOS.begin(); it != m_instancedVBOS.end(); it++) {
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

// Renders as many instances as specified in amount.
void vup::ParticleRenderer::execute(int amount)
{
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArraysInstanced(GL_TRIANGLES, 0, m_rdsize, amount);
  glBindVertexArray(0);
}
