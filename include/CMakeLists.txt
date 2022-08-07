set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(window)
add_subdirectory(imgui)
add_subdirectory(cli_emulator)
add_subdirectory(shaders)
add_subdirectory(cellmap)
add_subdirectory(rules)
add_subdirectory(renderer)

add_library(app_INC INTERFACE app.hpp)
target_include_directories(app_INC INTERFACE ${INCLUDE_DIR} "${CMAKE_BINARY_DIR}/shaders")
target_link_libraries(app_INC
  INTERFACE
    window_IMPL_glfw3
    shaders_IMPL
    renderer_IMPL
    cellsim_cli_emulator_IMPL
    rule_IMPL
    rule_config_IMPL
)
