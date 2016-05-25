#include "VBOHandler.h"

vup::VBOHandler::VBOHandler(int size)
{
  m_size = size;

  // Store pos data in an array buffer
  glGenBuffers(1, &m_posVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vup::particle::pos) * m_size, NULL, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &m_typeVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_typeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vup::particle::type) * m_size, NULL, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //m_vbos.push_back(std::make_pair(m_typeVBO, 1));

  glGenBuffers(1, &m_colorVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vup::particle::color) * m_size, NULL, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  m_vbos.push_back(std::make_pair(m_colorVBO, 4));
}

vup::VBOHandler::~VBOHandler()
{
}

void vup::VBOHandler::updatePositions(std::vector<vup::particle::pos>* data)
{
  glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
  vup::particle::pos * vertexArray = (vup::particle::pos *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (int i = 0; i < data->size(); i++) {
    vertexArray[i] = data->at(i);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void vup::VBOHandler::updateColor(std::vector<vup::particle::color>* data)
{
  glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
  vup::particle::color * vertexArray = (vup::particle::color *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (int i = 0; i < data->size(); i++) {
    vertexArray[i] = data->at(i);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void vup::VBOHandler::updateType(std::vector<vup::particle::type>* data)
{
  glBindBuffer(GL_ARRAY_BUFFER, m_typeVBO);
  vup::particle::type * vertexArray = (vup::particle::type *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (int i = 0; i < data->size(); i++) {
    vertexArray[i] = data->at(i);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
