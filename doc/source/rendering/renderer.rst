Rendering of Particles
===================================================================

Particles can be rendered using instanced rendering, which is implemented in :cpp:class:`vup::ParticleRenderer`. Instanced rendering is used to render the same data multiple times in a row. This is exceptionally useful for the rendering of particles because it is both more performant for a huge amount of objects to render and easier to execute than multiple draw calls.
Of course a :cpp:class:`vup::ShaderProgram` has to be created and used with a shader meant for instanced rendering to actual see the expected rendering result.

Example usage
******************

.. literalinclude:: ../../../examples/cl_instanced_renderer_example/cl_instanced_renderer_example.cpp

ParticleRenderer
******************

.. cpp:class:: vup::ParticleRenderer

    To use the ParticleRenderer, include :code:`vup/Rendering/ParticleRenderer.h`

    .. cpp:function:: ParticleRenderer(vup::RenderData rd, std::map<std::string, vup::VBO> instancedVBOs)
    
        :param rd: Vertices and normals of the reference object to be rendered
        :param instancedVBOs: Map of VBOs containing additional data unique to every rendered instance of the RenderData

    .. cpp:function:: execute()

    .. cpp:member:: int m_renderDataSize
