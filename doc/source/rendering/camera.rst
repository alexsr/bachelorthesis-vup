Virtual camera
===================================================================

To simulate view and projection of a virtual 3D world a virtual camera is used. There are different types of virtual cameras such as trackball or a first-person camera.

Trackball camera
*************************

A trackball camera is a camera revolving on a sphere with variable radius around a fixed center point in the world. It is however not possible to move the camera over the poles.

.. cpp:class:: vup::TrackballCam

    .. cpp:function:: TrackballCam(int width, int height, float sens, float r, float zoomsens, glm::vec3 center, float fov, float near, float far)

        Create an instance of the TrackballCam, save camera parameters and save initial camera pose and projection.

        :param int width: width of the display window
        :param int height: height of the display window
        :param float sens: sensitivity of mouse movement
        :param float r: initial radius of sphere the camera is moving on
        :param float zoomsens: sensitivity of zoom
        :param glm\:\:vec3 center: center of the trackball sphere
        :param float fov: field of view in degree
        :param float near: near value for projection
        :param float far: far value for projection

    .. cpp:function:: glm::mat4 getView()

        :return: camera view matrix

    .. cpp:function:: glm::mat4 getProjection()

        :return: camera projection matrix

    .. cpp:function:: void update(GLFWwindow* window, float dt)

        Use mouse movement to update camera position on the sphere and keyboard input to increase and decrease radius of said sphere. The left mouse button has to be held to register as movement.

        :param GLFWwindow* window: used for access to mouse movement and keyboard input
        :param float dt: update interval delta time
