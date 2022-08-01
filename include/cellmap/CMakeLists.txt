add_library(cellmap_INC INTERFACE cellmap.hpp)
target_include_directories(cellmap_INC INTERFACE ${INCLUDE_DIR} "${CMAKE_BINARY_DIR}/shaders")
