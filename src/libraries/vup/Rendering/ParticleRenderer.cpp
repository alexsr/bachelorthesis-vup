#include "ParticleRenderer.h"

vup::ParticleRenderer::ParticleRenderer(RenderData rd, std::map<std::string, vup::VBO> instancedVBOs)
{
  m_renderData = rd;
  m_renderDataSize = rd.getSize();

  m_instancedVBOs = instancedVBOs;

  // Create vbo for rendering using specified RenderData.
  GLuint renderVBO[2];
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(2, renderVBO);
  glBindVertexArray(m_vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, renderVBO[0]);
  glBufferData(GL_ARRAY_BUFFER, m_renderData.sizeOfVertices(), &(m_renderData.getVertices())[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, renderVBO[1]);
  glBufferData(GL_ARRAY_BUFFER, m_renderData.sizeOfNormals(), &(m_renderData.getNormals())[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

// Renders as many instances as specified in amount.
void vup::ParticleRenderer::execute(int amount)
{
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArraysInstanced(GL_TRIANGLES, 0, m_renderDataSize, amount);
  glBindVertexArray(0);
}
