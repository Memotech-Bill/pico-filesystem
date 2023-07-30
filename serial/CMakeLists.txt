if (NOT TARGET serial_filesystem)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(serial_filesystem INTERFACE)

  target_include_directories(serial_filesystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

  target_sources(serial_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_serial.c
    ${CMAKE_CURRENT_LIST_DIR}/picoser.c
    )

  target_link_libraries(sdcard_filesystem INTERFACE pico_filesystem)
  
endif()
