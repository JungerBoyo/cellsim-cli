find_package(glfw3  REQUIRED)
find_package(glad   REQUIRED)
find_package(spdlog REQUIRED)
find_package(CLI11  REQUIRED)
find_package(fmt    REQUIRED)
find_package(imgui  REQUIRED)

add_subdirectory(cli_emulator)
add_subdirectory(window)
add_subdirectory(imgui)
add_subdirectory(shaders)
add_subdirectory(cellmap)
add_subdirectory(renderer)
add_subdirectory(rules)

add_compile_definitions(GL_DEBUG)

add_library(app_IMPL STATIC app.cpp)
target_link_libraries(app_IMPL
  PUBLIC
    app_INC
  PRIVATE
    rule_info_window_IMPL
    imgui_utils_IMPL
)

if(GL_DEBUG)
  target_link_system_libraries(app_IMPL
    PRIVATE
      imgui::imgui
      glfw::glfw
      glad::glad
      spdlog::spdlog
  )
else()
  target_link_system_libraries(app_IMPL
    PRIVATE
      imgui::imgui
      glfw::glfw
  )
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_system_libraries(${PROJECT_NAME}
  PRIVATE
    spdlog::spdlog
)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    window_IMPL_glfw3
    app_IMPL
)

add_dependencies(${PROJECT_NAME} compile_and_copy_shaders_bin)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_BINARY_DIR}/config"
    "${CMAKE_BINARY_DIR}/shconfig"
)
