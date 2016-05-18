// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SHADERPROGRAM_H
#define VUP_SHADER_PROGRAM_H

#include "vup/defs.h"
#include "vup/Exceptions/ProgramCompilationException.h"
#include "vup/Exceptions/ShaderCompilationException.h"
#include "vup/Exceptions/FileNotFoundException.h"
#include "vup/Exceptions/UniformNotFoundException.h"
#include <string>
#include <fstream>
#include <iostream>

namespace vup {

// Creates an OpenGL Shader Program from source and provides
// additional functionality to use and update the shader
// inside the renderloop.

class ShaderProgram
{
public:
  ShaderProgram(const char* vertpath, const char* fragpath);
  ~ShaderProgram();
  // Activates the use of the shader program.
  void use();
  GLuint getProgram();
  void updateUniform(const GLchar* name, glm::mat4 m);

private:
  GLint findUniform(const GLchar* name);

  GLuint createShader(const char* path, GLenum type);

  // Loads the shader file source from the path but does not return the source.
  // Instead it adds the source to the shader with the given shaderID.
  void loadFromSource(const char* path, GLuint shaderID);

  // Checks if the shader was compiled correctly.
  void checkShaderStatus(GLuint shaderID);

  // Checks if the program was compiled correctly.
  void checkProgramStatus(GLuint programID);

  GLuint m_program;
};


}

#endif
