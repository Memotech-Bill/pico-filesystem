if (NOT TARGET sdcard_filesystem)

  cmake_policy(SET CMP0079 NEW)
  
  add_library(sdcard_filesystem INTERFACE)

  target_include_directories(sdcard_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../fatfs
    )

  pico_generate_pio_header(sdcard_filesystem ${CMAKE_CURRENT_LIST_DIR}/sd_spi.pio)

  target_sources(sdcard_filesystem INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pfs_fat.c
    ${CMAKE_CURRENT_LIST_DIR}/ff_disk.c
    ${CMAKE_CURRENT_LIST_DIR}/sd_spi2.c
    ${CMAKE_CURRENT_LIST_DIR}/../fatfs/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/../fatfs/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/../fatfs/ffunicode.c
    )

  target_link_libraries(sdcard_filesystem INTERFACE
    pico_filesystem
    hardware_gpio
    hardware_pio
    hardware_dma
    )
  
endif()
