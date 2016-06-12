#include "Renderer.h"

vup::Renderer::Renderer(RenderData rd)
{
  m_renderData = rd;
  m_renderDataSize = rd.getSize();

  // Create vbo for rendering using specified RenderData.
  GLuint renderVBO[2];
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(2, renderVBO);
  glBindVertexArray(m_vao);
  // Set vertices as attrib
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, renderVBO[0]);
  glBufferData(GL_ARRAY_BUFFER, m_renderData.sizeOfVertices(), &(m_renderData.getVertices())[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  // Set normals as attrib
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, renderVBO[1]);
  glBufferData(GL_ARRAY_BUFFER, m_renderData.sizeOfNormals(), &(m_renderData.getNormals())[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
}

vup::Renderer::~Renderer()
{
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &m_vao);
}

void vup::Renderer::execute()
{
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, m_renderDataSize);
  glBindVertexArray(0);
}
