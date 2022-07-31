if(REMOTE_BUILD)
  add_custom_target(compile_and_copy_shaders_bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/shconfig.hpp ${CMAKE_CURRENT_BINARY_DIR}/shconfig.hpp
  )
else()
  if(UNIX AND NOT APPLE)
    set(COMPILE_SHADERS make -C ${CMAKE_CURRENT_SOURCE_DIR})
  else()
    set(COMPILE_SHADERS ninja -C ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  add_custom_target(compile_and_copy_shaders_bin
    COMMAND ${COMPILE_SHADERS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/shconfig.hpp ${CMAKE_CURRENT_BINARY_DIR}/shconfig.hpp
  )
endif()