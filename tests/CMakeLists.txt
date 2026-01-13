# ==============================================================================
# Netra Tests
# ==============================================================================
add_executable(netra_tests
    src/test_main.cpp
    src/test_ecs.cpp
    src/test_simulation.cpp
)

target_link_libraries(netra_tests PRIVATE
    netra_engine
)

target_include_directories(netra_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
