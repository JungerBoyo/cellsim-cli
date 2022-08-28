find_package(imgui REQUIRED)
find_package(fmt REQUIRED)

add_library(cli_emulator_IMPL STATIC cli_emulator.cpp)

target_link_libraries(cli_emulator_IMPL
  PUBLIC
    cli_emulator_INC
  PRIVATE
    imgui_utils_IMPL
)

target_link_system_libraries(cli_emulator_IMPL
  PRIVATE
    imgui::imgui
)


add_library(cellsim_cli_emulator_IMPL STATIC cellsim_cli_emulator.cpp)

target_include_directories(cellsim_cli_emulator_IMPL
  PRIVATE
    "${CMAKE_BINARY_DIR}/config"
)

target_link_libraries(cellsim_cli_emulator_IMPL
  PUBLIC
    cellsim_cli_emulator_INC
  PRIVATE
    cli_emulator_IMPL
    rule_config_INC
)

target_link_system_libraries(cellsim_cli_emulator_IMPL
  PRIVATE
    fmt::fmt
)


