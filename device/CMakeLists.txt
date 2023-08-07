# The device filesystem

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

# Generic Output Driver

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

# Generic Input / Output Driver

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

# UART Driver

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

# USB Keyboard driver

if (NOT TARGET pfs_dev_kbd)

  set(EXTEND_ASCII      0)      # Set this to 1 to assign codes to non-printing keys
  set(USE_ASYNC_CONTEXT 1)
  set(DEBUG             0)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(pfs_dev_kbd INTERFACE)

  target_include_directories(pfs_dev_kbd INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/usb_host
    )

  target_compile_options(pfs_dev_kbd INTERFACE
    -DEXTEND_ASCII=${EXTEND_ASCII}
    -DUSE_ASYNC_CONTEXT=${USE_ASYNC_CONTEXT}
    -DDEBUG=${DEBUG}
    )

  target_sources(pfs_dev_kbd INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_kbd.c
    )

  if("${USE_ASYNC_CONTEXT}" STREQUAL "1")
    target_link_libraries(pfs_dev_kbd INTERFACE
      pico_async_context_threadsafe_background
      )
  endif()
  
  target_link_libraries(pfs_dev_kbd INTERFACE
    device_filesystem
    tinyusb_host
    tinyusb_board
    )

endif()

if (NOT TARGET pfs_dev_keymap_uk)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(pfs_dev_keymap_uk INTERFACE)

  target_sources(pfs_dev_keymap_uk INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_keymap_uk.c
    )

  target_link_libraries(pfs_dev_keymap_uk INTERFACE
    pfs_dev_kbd
    )

endif()
