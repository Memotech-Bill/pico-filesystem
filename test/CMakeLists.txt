set(PROJECT pfs_test)

set(HAVE_LFS         1)
set(HAVE_FAT         1)
set(HAVE_DEV         1)
set(HAVE_GIO         1)
set(ROOT_OFFSET      0x00070000)
set(ROOT_SIZE        65536)
if(NOT DEFINED SD_CLK_PIN)
  set(PICO_SD_CLK_PIN  5)
  set(PICO_SD_CMD_PIN  18)
  set(PICO_SD_DAT0_PIN 19)
  set(PICO_SD_DAT3_PIN 22)
endif()

cmake_minimum_required(VERSION 3.12)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(${PROJECT} C CXX ASM)
pico_sdk_init()
add_executable(${PROJECT} pfs_test.c)
target_compile_options(${PROJECT} PRIVATE -O1 -g)
target_link_options(${PROJECT} PRIVATE -g)
target_link_libraries(${PROJECT} pico_stdlib pico_binary_info)
pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)
# pico_set_binary_type(${PROJECT} no_flash)

target_compile_options(${PROJECT} PUBLIC -DSTATIC=)

target_compile_options(${PROJECT} PRIVATE
    -DHAVE_LFS=${HAVE_LFS}
    -DHAVE_FAT=${HAVE_FAT}
    -DHAVE_DEV=${HAVE_DEV}
    -DHAVE_GIO=${HAVE_GIO}
    -DROOT_OFFSET=${ROOT_OFFSET}
    -DROOT_SIZE=${ROOT_SIZE}
    -DPICO_SD_CLK_PIN=${PICO_SD_CLK_PIN}
    -DPICO_SD_CMD_PIN=${PICO_SD_CMD_PIN}
    -DPICO_SD_DAT0_PIN=${PICO_SD_DAT0_PIN}
    -DPICO_SD_DAT3_PIN=${PICO_SD_DAT3_PIN}
)
add_subdirectory (.. pfs)
if("${HAVE_LFS}" STREQUAL "1")
    target_link_libraries(${PROJECT} flash_filesystem)
endif()
if("${HAVE_FAT}" STREQUAL "1")
    target_link_libraries(${PROJECT} sdcard_filesystem)
endif()
if("${HAVE_DEV}" STREQUAL "1")
    target_link_libraries(${PROJECT} device_filesystem)
endif()
