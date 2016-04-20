#ifndef VUP_SHADERPROGRAM_H
#define VUP_SHADER_PROGRAM_H

#include "vup/defs.h"
#include <string>
#include <fstream>
#include <iostream>

namespace vup {

class ShaderProgram
{
public:
  ShaderProgram(const char* vertpath, const char* fragpath);
  ~ShaderProgram();
  void use();
  GLuint getProgram();

private:
  GLuint createShader(const char* path, GLenum type);
  void loadFromSource(const char* path, GLuint shader);
  void checkShaderStatus(GLuint shader);
  void checkProgramStatus(GLuint program);

  GLuint m_program;
};


}

#endif
