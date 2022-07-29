find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(hello_world)

add_executable(${PROJECT_NAME} 
  main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE 
    project_options
    project_warnings
    hello_world_IMPL
    glfw::glfw
    glad::glad
    spdlog::spdlog
    CLI11::CLI11
    fmt::fmt
)

add_dependencies(${PROJECT_NAME} compile_and_copy_shaders_bin)

target_include_directories(${PROJECT_NAME} 
  PRIVATE 
    "${CMAKE_BINARY_DIR}/config" 
)
