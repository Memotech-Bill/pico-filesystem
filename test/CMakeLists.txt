# Build pico-filesystem test programs

set(NO_FLASH         0)
set(HAVE_LFS         1)
set(HAVE_FAT         1)
set(HAVE_DEV         1)
set(HAVE_GIO         1)
set(ROOT_OFFSET      0x00070000)
set(ROOT_SIZE        65536)

cmake_minimum_required(VERSION 3.12)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pfs_test C CXX ASM)
pico_sdk_init()
if(NOT DEFINED SD_CLK_PIN)
  set(PICO_SD_CLK_PIN  5)
  set(PICO_SD_CMD_PIN  18)
  set(PICO_SD_DAT0_PIN 19)
  set(PICO_SD_DAT3_PIN 22)
endif()

# Test main pico-filesystem features

add_executable(pfs_test pfs_test.c)

target_compile_options(pfs_test PRIVATE -O1 -g)

target_compile_options(pfs_test PUBLIC -DSTATIC=)

target_compile_options(pfs_test PRIVATE
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
target_link_options(pfs_test PRIVATE -g)
target_link_libraries(pfs_test pico_stdlib pico_binary_info)

pico_add_extra_outputs(pfs_test)
pico_enable_stdio_usb(pfs_test 1)
pico_enable_stdio_uart(pfs_test 0)

if("${NO_FLASH}" STREQUAL "1")
  pico_set_binary_type(pfs_test no_flash)
endif()
add_subdirectory (.. pfs)
if("${HAVE_LFS}" STREQUAL "1")
    target_link_libraries(pfs_test flash_filesystem)
endif()
if("${HAVE_FAT}" STREQUAL "1")
    target_link_libraries(pfs_test sdcard_filesystem)
endif()
if("${HAVE_DEV}" STREQUAL "1")
    target_link_libraries(pfs_test device_filesystem)
endif()

# Test USB keyboard driver

add_executable(kbd_test kbd_test.c)

target_compile_options(kbd_test PRIVATE -O1 -g)

target_compile_options(kbd_test PUBLIC -DSTATIC=)
target_link_options(kbd_test PRIVATE -g)
target_link_libraries(kbd_test
  pico_stdlib
  pico_binary_info
  pfs_dev_kbd
  pfs_dev_keymap_uk
  )

pico_add_extra_outputs(kbd_test)
pico_enable_stdio_usb(kbd_test 0)
pico_enable_stdio_uart(kbd_test 1)
