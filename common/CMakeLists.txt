# ==============================================================================
# Netra Common Library
# ==============================================================================
add_library(netra_common INTERFACE)

target_include_directories(netra_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(netra_common INTERFACE cxx_std_23)
