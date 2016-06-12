Data for rendering
===================================================================

To render anything render data such as vertices and normals are necessary. This data can be stored in a :cpp:class:`vup::RenderData` object. This should probably mostly be used for primitive types such as spheres and cubes, whereas more complex objects should be integrated in a different manner i.e. the assimp loader.

Abstract RenderData class
*************************

.. cpp:class:: vup::RenderData

    This is the abstract class storing vertices and normals as well as mandatory functions every render data has in common.

    .. cpp:function:: std::vector<glm::vec4> getVertices()

        :returns: m_vertices - Vector of vertices
    
    .. cpp:function:: std::vector<glm::vec3> getNormals()

        :returns: m_normals - Vector of normals

    .. cpp:function:: int sizeOfVertices()

        Return the byte size of the m_vertices vector

    .. cpp:function:: int sizeOfNormals()

        Return the byte size of the m_normals vector

    .. cpp:function:: int getSize()

        :returns: m_size - amount of vertices/normals

Included geometric primitives
******************************

These are the primitives already implemented in the framework. To create a new primitive simply create a class derived from :cpp:class:`vup::RenderData`.

Sphere data
-------------

Spheres are the common primitive for rendering particles, which is why they are included here.

.. cpp:class:: vup::SphereData : public vup::RenderData

    .. cpp:function:: SphereData(float r, int hres, int vres)

        Create vertices and normals of a sphere with the specified parameters. The points are created using the sphere equation. This data is created for use with :code:`glDrawArrays()` as there are no indices defined.

        :param float r: Radius of the sphere
        :param int hres: Horizontal resolution
        :param int vres: Vertical resolution
    