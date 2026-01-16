# ==============================================================================
# Netra Graphics Library
# ==============================================================================
add_library(netra_graphics STATIC
    src/renderer.cpp
    src/shader.cpp
    src/window.cpp
    src/imgui_layer.cpp
)

target_include_directories(netra_graphics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(netra_graphics PUBLIC
    netra_common
    imgui
    glad
    glfw
    glm
    OpenGL::GL
)
