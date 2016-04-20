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
}

vup::ShaderProgram::~ShaderProgram()
{
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
  GLuint shader = glCreateShader(type);
  loadFromSource(path, shader);
  glCompileShader(shader);
  checkShaderStatus(shader);
  return shader;
}

void vup::ShaderProgram::loadFromSource(const char * path, GLuint shader)
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
  } else {
    std::cout << "ERROR: Failed loading shader source from " << path << std::endl;
  }
  const char* shaderSource = source.c_str();
  GLint sourceSize = strlen(shaderSource);
  glShaderSource(shader, 1, &shaderSource, &sourceSize);
}

void vup::ShaderProgram::checkShaderStatus(GLuint shader)
{
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* infoLog = new GLchar[infoLogLength + 1];
    glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
    std::cout << "ERROR: Shader compilation failed.\n" << infoLog << std::endl;
    delete[] infoLog;
  }
}

void vup::ShaderProgram::checkProgramStatus(GLuint program)
{
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* infoLog = new GLchar[infoLogLength + 1];
    glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
    std::cout << "ERROR: Shader program creation failed.\n" << infoLog << std::endl;
    delete[] infoLog;
  }
}
