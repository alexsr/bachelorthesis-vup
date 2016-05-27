#include "ShaderProgram.h"

vup::ShaderProgram::ShaderProgram(const char* vertpath, const char* fragpath)
{
  GLuint vertexShader = createShader(vertpath, GL_VERTEX_SHADER);
  GLuint fragmentShader = createShader(fragpath, GL_FRAGMENT_SHADER);

  m_program = glCreateProgram();
  glAttachShader(m_program, vertexShader);
  glAttachShader(m_program, fragmentShader);
  glLinkProgram(m_program);
  checkProgramStatus(m_program);

  // Delete shaders because they are already compiled into the program
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

vup::ShaderProgram::~ShaderProgram()
{
  // Unbind the shader program in case it is in use before deleting it.
  glUseProgram(0);
  glDeleteProgram(m_program);
}

void vup::ShaderProgram::use()
{
  glUseProgram(m_program);
}

GLuint vup::ShaderProgram::getProgram()
{
  return m_program;
}

void vup::ShaderProgram::updateUniform(const GLchar* name, bool b)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform1i(loc, b);
}

void vup::ShaderProgram::updateUniform(const GLchar* name, int i)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform1i(loc, i);
}

void vup::ShaderProgram::updateUniform(const GLchar* name, float f)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform1f(loc, f);
}

void vup::ShaderProgram::updateUniform(const GLchar* name, double d)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform1f(loc, d);
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::vec2 v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform2fv(loc, 1, glm::value_ptr(v));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::vec3 v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform3fv(loc, 1, glm::value_ptr(v));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::vec4 v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform4fv(loc, 1, glm::value_ptr(v));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::ivec2 v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform2iv(loc, 1, glm::value_ptr(v));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::ivec3 v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform3iv(loc, 1, glm::value_ptr(v));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::ivec4 v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform4iv(loc, 1, glm::value_ptr(v));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, std::vector<glm::vec2> v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform2fv(loc, sizeof(v), glm::value_ptr((&v[0])[0]));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, std::vector<glm::vec3> v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform3fv(loc, sizeof(v), glm::value_ptr((&v[0])[0]));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, std::vector<glm::vec4> v)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniform4fv(loc, sizeof(v), glm::value_ptr((&v[0])[0]));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::mat2 m)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::mat3 m)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void vup::ShaderProgram::updateUniform(const GLchar* name, glm::mat4 m)
{
  GLint loc = findUniform(name);
  glUseProgram(m_program);
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

GLint vup::ShaderProgram::findUniform(const GLchar* name)
{
  GLint loc = glGetUniformLocation(m_program, name);
  if (loc == -1) {
    throw(vup::UniformNotFoundException(name));
  }
  return loc;
}

GLuint vup::ShaderProgram::createShader(const char * path, GLenum type)
{
  GLuint shaderID = glCreateShader(type);
  loadFromSource(path, shaderID);
  glCompileShader(shaderID);
  checkShaderStatus(shaderID);
  return shaderID;
}

void vup::ShaderProgram::loadFromSource(const char * path, GLuint shaderID)
{
  vup::FileReader fr(path);
  if (fr.isLoaded()) {
    std::cout << "SUCCESS: Loading shader source from " << path << std::endl;
  }
  const char* shaderSource = fr.getSourceChar();
  GLint sourceSize = strlen(shaderSource);
  glShaderSource(shaderID, 1, &shaderSource, &sourceSize);
}

void vup::ShaderProgram::checkShaderStatus(GLuint shaderID)
{
  GLint status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* infoLog = new GLchar[infoLogLength + 1];
    glGetShaderInfoLog(shaderID, infoLogLength, NULL, infoLog);
    // When throwing the compilation exception,
    // the info log from the compiler is included.
    throw(vup::ShaderCompilationException(infoLog));
    delete[] infoLog;
  }
  else {
    std::cout << "SUCCESS: Compiling shader." << std::endl;
  }
}

void vup::ShaderProgram::checkProgramStatus(GLuint programID)
{
  GLint status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* infoLog = new GLchar[infoLogLength + 1];
    glGetProgramInfoLog(programID, infoLogLength, NULL, infoLog);
    // When throwing the compilation exception,
    // the info log from the compiler is included.
    throw(vup::ProgramCompilationException(infoLog));
    delete[] infoLog;
  }
  else {
    std::cout << "SUCCESS: Compiling shader program." << std::endl;
  }
}
