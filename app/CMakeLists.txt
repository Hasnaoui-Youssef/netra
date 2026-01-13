# ==============================================================================
# Netra Application
# ==============================================================================
add_executable(netra
    src/main.cpp
)

target_link_libraries(netra PRIVATE
    netra_engine
    netra_graphics
)
