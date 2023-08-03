if (NOT TARGET device_filesystem)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(device_filesystem INTERFACE)

  target_include_directories(device_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    )

  target_sources(device_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev.c
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_uart.c
    )

  target_link_libraries(device_filesystem INTERFACE
    pico_filesystem
    pico_stdio
    )

endif()
