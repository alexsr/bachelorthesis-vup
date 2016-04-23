cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0022 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 OLD)
endif()

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

include_directories(
    ${OPENGL_INCLUDE_PATH}
    ${GLEW_INCLUDE_PATH}
    ${GLFW3_INCLUDE_PATH}
    ${GLM_INCLUDE_PATH}
    ${ASSIMP_INCLUDE_DIRS}
    ${OPENCL_INCLUDE_DIRS}
    ${EXTERNAL_LIBRARY_PATHS}
    ${LIBRARIES_PATH}
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADER *.h)

add_definitions(-DSHADERS_PATH="${SHADERS_PATH}")
add_definitions(-DOPENCL_KERNEL_PATH="${OPENCL_KERNEL_PATH}")
#add_definitions(-DRESOURCES_PATH="${RESOURCES_PATH}")
add_definitions(-DGLFW_INCLUDE_GLCOREARB)
add_definitions(-DGLEW_STATIC)

add_library(${ProjectId} ${SOURCES} ${HEADER})

target_link_libraries(
    ${ProjectId}
    ${ALL_LIBRARIES}
    ${GLFW3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    ${OPENCL_LIBRARIES}
)

set_target_properties(${ProjectId} PROPERTIES LINKER_LANGUAGE CXX)