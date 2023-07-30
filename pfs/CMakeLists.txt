if (NOT TARGET pico_filesystem)
  
  pico_add_library(pico_filesystem)

  target_include_directories(pico_filesystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

  target_sources(pico_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_base.c
    ${CMAKE_CURRENT_LIST_DIR}/pfs_stdio.c
    ${CMAKE_CURRENT_LIST_DIR}/pname.c
    )
  
endif()
