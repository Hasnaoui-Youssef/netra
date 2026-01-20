# ==============================================================================
# Netra Application
# ==============================================================================
add_executable(netra
    src/main.cpp
    src/gate_editor.cpp
)

target_include_directories(netra PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(netra PRIVATE
    netra_engine
    imgui
)
