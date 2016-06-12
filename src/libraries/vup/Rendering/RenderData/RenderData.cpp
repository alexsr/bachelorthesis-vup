#include "RenderData.h"

vup::RenderData::RenderData()
{
  m_size = 0;
  m_vertices = std::vector<glm::vec4>();
  m_normals = std::vector<glm::vec3>();
}

vup::RenderData::~RenderData()
{

}
