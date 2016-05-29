// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BUFFERHANDLER_H
#define VUP_BUFFERHANDLER_H

#include "vup/defs.h"
#include "vup/particle.h"
#include "vup/Rendering/VBO.h"
#include <vector>
#include <map>
#include <iostream>

namespace vup {

class BufferHandler
{
public:
  BufferHandler();
  ~BufferHandler();
  template <typename T> void createVBO(std::string name, int loc, int size, int format, bool isInterop = false, GLint drawType = GL_STATIC_DRAW);
  template <typename T> void createVBOData(std::string name, int loc, int size, int format, std::vector<T> data, bool isInterop = false, GLint drawType = GL_STATIC_DRAW);
  template <typename T> void updateVBO(std::string name, std::vector<T> data);
  template <typename T> void updateSubVBO(std::string name, std::vector<T> data, int range);
  std::map<std::string, vup::VBO> getVBOs() { return m_vbos; }
  std::map<std::string, vup::VBO> getInteropVBOs() { return m_interopVBOs; }
  vup::VBO getVBO(std::string name);
  vup::VBO getInteropVBO(std::string name);
  GLuint getVBOHandle(std::string name);
  GLuint getInteropVBOHandle(std::string name);

private:
  std::map<std::string, vup::VBO> m_vbos;
  std::map<std::string, vup::VBO> m_interopVBOs;
};

}


#endif
