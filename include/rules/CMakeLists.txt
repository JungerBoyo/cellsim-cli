add_library(rule_INC INTERFACE rule.hpp)
add_library(rule_config_INC INTERFACE rule_config.hpp)
add_library(rule_info_window_INC INTERFACE rule_info_window.hpp)

target_include_directories(rule_INC INTERFACE ${INCLUDE_DIR})
target_include_directories(rule_config_INC INTERFACE ${INCLUDE_DIR})
target_include_directories(rule_info_window_INC INTERFACE ${INCLUDE_DIR})