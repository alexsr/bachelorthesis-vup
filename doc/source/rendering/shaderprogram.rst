.. highlight:: cpp

Shaders
===================================================================

Shaders are used to alter rendering of vertices. Vertex shaders manipulate vertices of a model while fragment shaders eventually create the image on the screen by assigning color to pixels. The shader handler simplifies the loading and using of shaders significantly ny encapsulating OpenGL methods.

Shaders should reside in the :code:`src/shaders/` directory which has the defined variable :code:`SHADERS_PATH` associated with it to use in files in the project.

Shader program handler
****************************

.. cpp:class:: vup::ShaderProgram

    To use the ShaderProgram, include :code:`vup/Rendering/ShaderProgram.h`. The ShaderProgram needs both a vertex and a fragment shader written in GLSL.

    .. cpp:function:: ShaderProgram(const char* vertpath, const char* fragpath)

        Create an instance of ShaderProgram with a vertex and a fragment shader. These shaders are compiled and attached to a program. Through the process of compiling, shaders are checked for compilation errors to ensure functionality.
    
        :param const char* vertpath: Path to the vertex shader
        :param const char* fragpath: Path to the fragment shader

    .. cpp:function:: void use()
        
        Use the shader program :code:`m_program` defined in the class instance.

    .. cpp:function:: GLuint getProgram()

        :returns: m_program - id of the shader program

    .. cpp:function:: void updateUniform(const GLchar * name, bool b)

        Write b to a boolean uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param bool b: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, int i)

        Write i to an int uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param int i: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, float f)

        Write f to a float uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param float f: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, double d)

        Write d to a double uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param double d: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::vec2 v)

        Write a pointer to v to a vec2 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:vec2 v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::vec3 v)

        Write a pointer to v to a vec3 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:vec3 v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::vec4 v)

        Write a pointer to v to a vec4 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:vec4 v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::ivec2 v)

        Write a pointer to v to a ivec2 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:ivec2 v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::ivec3 v)

        Write a pointer to v to a ivec3 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:ivec3 v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::ivec4 v)

        Write a pointer to v to a ivec4 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:ivec4 v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, std::vector<glm::vec2> v)

        Write a pointer to (&v[0])[0] to a vec2 array uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param std\:\:vector<glm\:\:vec2> v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, std::vector<glm::vec3> v)

        Write a pointer to (&v[0])[0] to a vec3 array uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param std\:\:vector<glm\:\:vec3> v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, std::vector<glm::vec4> v)

        Write a pointer to (&v[0])[0] to a vec4 array uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param std\:\:vector<glm\:\:vec4> v: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::mat2 m)

        Write a pointer to m to a mat2 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:mat2 m: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::mat3 m)

        Write a pointer to m to a mat3 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:mat3 m: value to write to the uniform

    .. cpp:function:: void updateUniform(const GLchar * name, glm::mat4 m)

        Write a pointer to m to a mat4 uniform in shader with the specified name

        :param const GLchar * name: Name of the uniform in the shader
        :param glm\:\:mat4 m: value to write to the uniform

Example usage
******************

A shader could for example warp the vertices to fit the parameters of a camera. In this example, the view and projection of the camera would be used in the shader and the warped vertices would be rendered in a red color.

::

    [...]
    #include "vup/Rendering/ShaderProgram.h"

    int main() {
        [...]
        vup::ShaderProgram simpleShader(SHADERS_PATH "/viewProjection.vert", SHADERS_PATH "/redColor.frag");
        simpleShader.updateUniform("proj", cam.getProjection());
        [...]
        while (true) {
            [...]
            cam.update(window, camdt);
            simpleShader.updateUniform("view", cam.getView());
            simpleShader.use();
            [...]
        }
        [...]
    }

Minimal shader examples
************************

Vertex Shader
--------------

.. code-block:: c

    #version 330 core
    layout (location = 0) in vec4 position;
    layout (location = 1) in vec4 normal;
    
    uniform mat4 view;
    uniform mat4 proj;

    void main()
    {
        gl_Position = proj * view * position;
    }

Fragment Shader
----------------

.. code-block:: c

    #version 330 core
    out vec4 color;

    void main()
    {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }
