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
    )

endif()

if (NOT TARGET pfs_dev_gdd)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(pfs_dev_gdd INTERFACE)

  target_sources(device_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_gdd.c
    )

  target_link_libraries(device_filesystem INTERFACE
    device_filesystem
    )

endif()

if (NOT TARGET pfs_dev_gio)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(pfs_dev_gio INTERFACE)

  target_sources(device_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_gio.c
    )

  target_link_libraries(device_filesystem INTERFACE
    device_filesystem
    )

endif()

if (NOT TARGET pfs_dev_uart)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(pfs_dev_uart INTERFACE)

  target_sources(device_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_uart.c
    )

  target_link_libraries(device_filesystem INTERFACE
    device_filesystem
    )

endif()
