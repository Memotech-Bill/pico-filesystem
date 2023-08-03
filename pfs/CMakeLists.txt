if (NOT TARGET pico_filesystem)
  
  pico_add_library(pico_filesystem)

  target_include_directories(pico_filesystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

  target_sources(pico_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_base.c
    ${CMAKE_CURRENT_LIST_DIR}/pname.c
    ${CMAKE_CURRENT_LIST_DIR}/pfs_dev_tty.c
    )

  target_link_libraries(pico_filesystem INTERFACE
    pico_stdio
    pico_stdlib
    pico_malloc
    pico_mem_ops
    )
  
endif()
