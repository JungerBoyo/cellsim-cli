find_package(CLI11 REQUIRED)

add_library(cli_emulator_INC INTERFACE cli_emulator.hpp)
target_include_directories(cli_emulator_INC INTERFACE ${INCLUDE_DIR})

target_link_system_libraries(cli_emulator_INC
  INTERFACE
    CLI11::CLI11
)

add_library(cellsim_cli_emulator_INC INTERFACE cellsim_cli_emulator.hpp)
target_include_directories(cellsim_cli_emulator_INC INTERFACE ${INCLUDE_DIR})

target_link_libraries(cellsim_cli_emulator_INC
  INTERFACE
    cli_emulator_INC
)