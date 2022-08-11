find_package(glad REQUIRED)
find_package(imgui REQUIRED)

add_library(rule_IMPL STATIC rule.cpp)
add_library(rule_config_IMPL STATIC rule_config.cpp)
add_library(rule_info_window_IMPL STATIC rule_info_window.cpp)

target_link_libraries(rule_config_IMPL
  PUBLIC
    rule_config_INC
  PRIVATE
    shaders_IMPL
)

target_link_libraries(rule_IMPL
  PUBLIC
    rule_INC
  PRIVATE
    rule_config_IMPL
)

target_include_directories(rule_IMPL
  PRIVATE
    "${CMAKE_BINARY_DIR}/shaders"
)

target_link_system_libraries(rule_IMPL PRIVATE glad::glad)

target_link_libraries(rule_info_window_IMPL
  PUBLIC
    rule_info_window_INC
  PRIVATE
    rule_config_IMPL
    rule_IMPL
)

target_link_system_libraries(rule_info_window_IMPL PRIVATE imgui::imgui)
