// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SHADERPROGRAM_H
#define VUP_SHADER_PROGRAM_H

#include "vup/defs.h"
#include "vup/Exceptions/ShaderProgramCompilationException.h"
#include "vup/Exceptions/ShaderCompilationException.h"
#include "vup/Exceptions/UniformNotFoundException.h"
#include "vup/Util/FileReader.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

namespace vup {

// Creates an OpenGL Shader Program from source and provides
// additional functionality to use the shader and update the
// uniform values.

class ShaderProgram
{
public:
  ShaderProgram(const char* vertpath, const char* fragpath);
  ~ShaderProgram();
  // Activates the use of the shader program.
  void use();
  GLuint getProgram();

  // Updates the uniform at the location of name with
  // the second value in the method decleration.
  // These are the essential methods for updating uniforms.

  void updateUniform(const GLchar * name, bool b);
  void updateUniform(const GLchar * name, int i);
  void updateUniform(const GLchar * name, float f);
  void updateUniform(const GLchar * name, double d);
  void updateUniform(const GLchar * name, glm::vec2 v);
  void updateUniform(const GLchar * name, glm::vec3 v);
  void updateUniform(const GLchar * name, glm::vec4 v);
  void updateUniform(const GLchar * name, glm::ivec2 v);
  void updateUniform(const GLchar * name, glm::ivec3 v);
  void updateUniform(const GLchar * name, glm::ivec4 v);
  void updateUniform(const GLchar * name, std::vector<glm::vec2> v);
  void updateUniform(const GLchar * name, std::vector<glm::vec3> v);
  void updateUniform(const GLchar * name, std::vector<glm::vec4> v);
  void updateUniform(const GLchar * name, glm::mat2 m);
  void updateUniform(const GLchar * name, glm::mat3 m);
  void updateUniform(const GLchar* name, glm::mat4 m);

private:

  // Tries to find the uniform location of the uniform with the given name.
  // If the uniform is not found in the shader, an exception is thrown.
  GLint findUniform(const GLchar* name);

  // Handles the shader creation and returns the id of the compiled shader.
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
