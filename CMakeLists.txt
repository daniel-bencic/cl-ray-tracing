cmake_minimum_required(VERSION 3.16.3)
project(cl-ray-tracing)

add_definitions(-DPROGRAM_NAME="${CMAKE_PROJECT_NAME}" -DROOT_DIR="${CMAKE_SOURCE_DIR}" -DCL_TARGET_OPENCL_VERSION=120)

set(ROCM_OPENCL "/opt/rocm/opencl/")

find_library(CL OpenCL PATHS "${ROCM_OPENCL}lib/" REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
        src/main.c
        src/renderer.c
        src/ray_tracer.c
        src/util.c
        src/linear_algebra.c)

target_include_directories(${CMAKE_PROJECT_NAME} 
        PRIVATE include
        PRIVATE "${ROCM_OPENCL}include/")

target_link_libraries(${CMAKE_PROJECT_NAME} 
        m
        GL
        glfw
        GLEW
        ${CL})
