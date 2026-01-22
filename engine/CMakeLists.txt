# ==============================================================================
# Netra Engine Library
# ==============================================================================
add_library(netra_engine STATIC
    src/core/entity.cpp
    src/core/astar.cpp
    src/components/components.cpp
    src/components/render_components.cpp
    src/systems/simulation.cpp
    src/systems/layout_system.cpp
    src/systems/render_system.cpp
    src/graphics/shader.cpp
    src/graphics/window.cpp
    src/graphics/imgui_layer.cpp
    src/graphics/grid.cpp
    src/graphics/camera2d.cpp
)

target_include_directories(netra_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(netra_engine PUBLIC
    netra_common
    boost_headers
    imgui
    glad
    glfw
    glm
    OpenGL::GL
)
