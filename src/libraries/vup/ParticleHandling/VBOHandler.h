// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_VBOHANDLER_H
#define VUP_VBOHANDLER_H

#include "vup/defs.h"
#include "vup/particle.h"
#include <vector>

namespace vup {

class VBOHandler
{
public:
  VBOHandler(int size);
  ~VBOHandler();
  void updatePositions(std::vector<vup::particle::pos>* data);
  void updateColor(std::vector<vup::particle::color>* data);
  void updateType(std::vector<vup::particle::type>* data);
  GLuint getPosVBO() { return m_posVBO; }
  GLuint getTypeVBO() { return m_typeVBO; }
  GLuint getColorVBO() { return m_colorVBO; }
  std::vector<std::pair<GLuint, int>> getVBOs() { return m_vbos; }


private:
  int m_size;
  GLuint m_posVBO;
  GLuint m_typeVBO;
  GLuint m_colorVBO;
  std::vector<std::pair<GLuint, int>> m_vbos;
};

}

#endif
