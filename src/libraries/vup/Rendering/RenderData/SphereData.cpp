#include "SphereData.h"

vup::SphereData::SphereData(float r, int hres, int vres)
{
  m_hres = hres;
  m_vres = vres;
  m_radius = r;
  m_vertices.resize(hres * vres * 6);
  m_normals.resize(hres * vres * 6);
  float d_h = 2 * glm::pi<float>()/((float) hres);
  float d_v = glm::pi<float>()/((float) vres);
  
  int n = 0;

  // Vertices are created inside this loop.
  for (int i = 0; i < hres; i++) {
    float h = i * d_h;
    float hn = h + d_h;
    for (int j = 0; j < vres; j++) {
      float v = j * d_v;
      float vn = v + d_v;

      // The sphere is consists of multiple triangles
      // where 2 triangles make a plane.
      // These 4 points are the corners of said plane.
      // To create a triangle 3 of these corners are
      // used counterclockwise with the 2nd triangle's
      // first point being the 1st last point.
      // Normal vectors are the same as the points
      // without the radius multiplied.
      glm::vec4 p0 = createPoint(h, v);
      glm::vec4 p1 = createPoint(h, vn);
      glm::vec4 p2 = createPoint(hn, v);
      glm::vec4 p3 = createPoint(hn , vn);
      m_vertices[n] = p0 * r;
      m_normals[n++] = glm::vec3(p0);
      m_vertices[n] = p1 * r;
      m_normals[n++] = glm::vec3(p1);
      m_vertices[n] = p3 * r;
      m_normals[n++] = glm::vec3(p3);
      m_vertices[n] = p3 * r;
      m_normals[n++] = glm::vec3(p3);
      m_vertices[n] = p2 * r;
      m_normals[n++] = glm::vec3(p2);
      m_vertices[n] = p0 * r;
      m_normals[n++] = glm::vec3(p0);
    }
  }
  m_size = n;
}

vup::SphereData::~SphereData()
{

}

glm::vec4 vup::SphereData::createPoint(float theta, float phi)
{
  glm::vec4 p;
  p.x = glm::cos(theta) * glm::sin(phi);
  p.y = glm::sin(theta) * glm::sin(phi);
  p.z = glm::cos(phi);
  p.w = 1.0f;
  return p;
}
