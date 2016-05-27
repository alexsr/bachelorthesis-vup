#include "VBO.h"

vup::VBO::VBO()
{
  handle = 0;
  location = 0;
  size = 0;
}

vup::VBO::VBO(GLuint h, int loc, int s)
{
  handle = h;
  location = loc;
  size = s;
}

vup::VBO::~VBO()
{
}
