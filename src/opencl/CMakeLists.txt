project(KERNELS)

file(GLOB_RECURSE OPENCLKERNEL *.cl)
SOURCE_GROUP(OpenCL-Kernels FILES ${OPENCLKERNEL})

set(OPENCLKERNELS ${OPENCLKERNEL} PARENT_SCOPE)

add_custom_target(KERNELS SOURCES ${OPENCLKERNEL})
