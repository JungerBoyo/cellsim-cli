add_library(rule_INC INTERFACE rule.hpp)
add_library(rule_config_INC INTERFACE rule_config.hpp)

target_include_directories(rule_INC INTERFACE ${INCLUDE_DIR} "${CMAKE_BINARY_DIR}/shaders")
target_include_directories(rule_config_INC INTERFACE ${INCLUDE_DIR})