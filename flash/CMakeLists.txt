if (NOT TARGET flash_filesystem)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(flash_filesystem INTERFACE)

  target_include_directories(flash_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../littlefs
    )

  target_sources(flash_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_ffs.c
    ${CMAKE_CURRENT_LIST_DIR}/ffs_pico.c
    ${CMAKE_CURRENT_LIST_DIR}/../littlefs/lfs.c
    )

  target_link_libraries(flash_filesystem INTERFACE pico_filesystem)

endif()
