#include "SphereData.h"

vup::SphereData::SphereData(float r, int hres, int vres)
{
  m_hres = hres;
  m_vres = vres;
  m_r = r;
  m_vertices.resize(hres * vres * 6);
  m_normals.resize(hres * vres * 6);
  float d_h = 2 * glm::pi<float>()/((float) hres);
  float d_v = glm::pi<float>()/((float) vres);
  
  int n = 0;

  for (int i = 0; i < hres; i++) {
    float h = i * d_h;
    float hn = h + d_h;
    for (int j = 0; j < vres; j++) {
      float v = j * d_v;
      float vn = v + d_v;
      glm::vec3 p0 = createPoint(h, v);
      glm::vec3 p1 = createPoint(h, vn);
      glm::vec3 p2 = createPoint(hn, v);
      glm::vec3 p3 = createPoint(hn , vn);
      m_vertices[n] = p0 * r;
      m_normals[n++] = p0;
      m_vertices[n] = p1 * r;
      m_normals[n++] = p1;
      m_vertices[n] = p3 * r;
      m_normals[n++] = p3;
      m_vertices[n] = p3 * r;
      m_normals[n++] = p3;
      m_vertices[n] = p2 * r;
      m_normals[n++] = p2;
      m_vertices[n] = p0 * r;
      m_normals[n++] = p0;
    }

  }
}

vup::SphereData::~SphereData()
{

}

glm::vec3 vup::SphereData::createPoint(float theta, float phi)
{
  glm::vec3 p;
  p.x = glm::cos(theta) * glm::sin(phi);
  p.y = glm::sin(theta) * glm::sin(phi);
  p.z = glm::cos(phi);
  return p;
}
