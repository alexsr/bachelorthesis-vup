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
  std::string source = "";
  std::string line = "";
  std::ifstream shaderfile(path);
  if (shaderfile.is_open()) {
    while (std::getline(shaderfile, line)) {
      source += line + "\n";
    }
    shaderfile.close();
    std::cout << "SUCCESS: Loading shader source from " << path << std::endl;
  }
  else {
    throw(vup::FileNotFoundException(path));
  }
  const char* shaderSource = source.c_str();
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
    throw(vup::ProgramCompilationException(infoLog));
    delete[] infoLog;
  }
  else {
    std::cout << "SUCCESS: Compiling shader program." << std::endl;
  }
}
