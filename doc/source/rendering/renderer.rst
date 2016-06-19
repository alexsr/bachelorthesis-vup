.. highlight:: cpp

Renderers
===================================================================

To render any data whatsoever, renderer classes are implemented in the framework. If you want to create your own renderer, it might be beneficial but not mandatory to make it a derived class from :cpp:class:`vup::Renderer`.

Base Renderer
**************

This base renderer class holds all rendering data mutual to the other renderer classes. Mostly the way :cpp:func:`vup::Renderer::execute` is implemented might be different in derived classes.

.. cpp:class:: vup::Renderer

    .. cpp:function:: Renderer(vup::RenderData rd)

        Create an instance of the Renderer and fill the vertex array object with data. Both vertices and normals from the RenderData are added at location 0 and 1.

        :param vup\:\:RenderData rd: vertices and normals of the object to be rendered

    .. cpp:function:: Renderer()

            Create an instance of the Renderer and fill the vertex array object with data. Both vertices and normals from the RenderData are added at location 0 and 1.

            :param vup\:\:RenderData rd: vertices and normals of the object to be rendered

    .. cpp:function:: execute()

        Render the render data using :code:`glDrawArrays()`

Instanced Particle Renderer
****************************

Particles can be rendered using instanced rendering, which is implemented in :cpp:class:`vup::ParticleRenderer`. Instanced rendering is used to render the same data multiple times in a row. This is exceptionally useful for the rendering of particles because it is both more performant for a huge amount of objects to render and easier to execute than multiple draw calls.

Of course a :cpp:class:`vup::ShaderProgram` has to be created and used with a shader meant for instanced rendering to actual see the expected rendering result.

.. cpp:class:: vup::ParticleRenderer : public vup::Renderer

    To use the ParticleRenderer, include :code:`vup/Rendering/ParticleRenderer.h`. The ParticleRenderer relies on :cpp:class:`vup::RenderData` and :cpp:class:`vup::BufferHandler`.

    .. cpp:function:: ParticleRenderer(vup::RenderData rd, std::map<std::string, vup::VBO> instancedVBOs)

        Create an instance of the ParticleRenderer, invoke :cpp:func:`vup::Renderer::Renderer` and add every vertex buffer object in the map to the vertex array object at the location specified in the :cpp:class:`vup::VBO` and enabled as an instanced attribute by invoking :code:`glVertexAttribDivisor()`.
    
        :param vup\:\:RenderData rd: Vertices and normals of the reference object to be rendered
        :param std\:\:map<std\:\:string, vup\:\:VBO> instancedVBOs: Map of VBOs containing additional data unique to every rendered instance of the RenderData

    .. cpp:function:: void execute(int n)
        
        Render n instances of the RenderData using :code:`glDrawArraysInstanced()`

        :param int n: Number of instances of render data to be drawn

Example usage
--------------

Most commonly particles would be represented as spheres. Instanced VBO data is stored in a map in :cpp:class:`vup::BufferHandler` and accessible by calling :meth:`vup::BufferHandler::getInteropVBOs()`
::

    [...]
    #include "vup/Rendering/ParticleRenderer.h"

    int main() {
        [...]
        vup::SphereData sphere(0.1f, 20, 20);
        vup::ParticleRenderer renderer(sphere, buffers.getInteropVBOs());
        [...]
        while (true) {
            [...]
            shader.use();
            renderer.execute(particle_amount);
            [...]
        }
        [...]
    }

Minimal instanced shader example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This shader utilizes a VBO wherein a positional offset is defined. The fragment shader does not need any special behavior.

.. code-block:: c

    #version 330 core
    layout (location = 0) in vec4 position;
    layout (location = 1) in vec4 normal;
    // The offset and possibly other instance specific arguments can be specified in additional vbos.
    // These are the vbos passed to the particle renderer.
    layout (location = 2) in vec4 offset;

    uniform mat4 view;
    uniform mat4 proj;

    void main()
    {
        gl_Position = proj * view * (position + offset);
    }

.. seealso:: :cpp:class:`vup::RenderData`, :cpp:class:`vup::BufferHandler`
