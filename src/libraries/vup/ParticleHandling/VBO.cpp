#include "VBO.h"

vup::VBO::VBO()
{
  handle = 0;
  location = 0;
  format = 0;
}

vup::VBO::VBO(GLuint h, int loc, int f)
{
  handle = h;
  location = loc;
  format = f;
}

vup::VBO::~VBO()
{
}
