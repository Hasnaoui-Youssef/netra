# ==============================================================================
# Netra Engine Library
# ==============================================================================
add_library(netra_engine STATIC
    src/core/entity.cpp
    src/components/components.cpp
    src/systems/simulation.cpp
)

target_include_directories(netra_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(netra_engine PUBLIC
    netra_common
)
