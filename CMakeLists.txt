cmake_minimum_required(VERSION 3.20)
project(Netra C CXX)

# ------------------------------------------------------------------------------
# Build Configuration
# ------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Version
# ------------------------------------------------------------------------------
set(NETRA_VERSION_MAJOR 0)
set(NETRA_VERSION_MINOR 1)
set(NETRA_VERSION_PATCH 0)
set(NETRA_VERSION "${NETRA_VERSION_MAJOR}.${NETRA_VERSION_MINOR}.${NETRA_VERSION_PATCH}")
message(STATUS "Netra version: ${NETRA_VERSION}")

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(NETRA_BUILD_TESTS "Build tests" OFF)
option(NETRA_BUILD_DOCS "Build documentation" OFF)

# ------------------------------------------------------------------------------
# External Dependencies
# ------------------------------------------------------------------------------
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLAD (local)
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/glad/include/glad
    ${CMAKE_SOURCE_DIR}/glad/include
)

# ------------------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(graphics)
add_subdirectory(app)

if(NETRA_BUILD_TESTS)
    add_subdirectory(tests)
endif()

