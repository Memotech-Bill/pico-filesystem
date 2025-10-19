# Build pico-filesystem test programs

if(NOT DEFINED NO_FLASH)
    set(NO_FLASH         0)
endif()
if(NOT DEFINED HAVE_LFS)
    set(HAVE_LFS         1)
endif()
if(NOT DEFINED HAVE_FAT)
    set(HAVE_FAT         0)
endif()
if(NOT DEFINED HAVE_DEV)
    set(HAVE_DEV         0)
endif()
if(NOT DEFINED HAVE_UART)
    set(HAVE_UART        0)
endif()
if(NOT DEFINED HAVE_GIO)
    set(HAVE_GIO         0)
endif()
if(NOT DEFINED ROOT_OFFSET)
    set(ROOT_OFFSET      0x00070000)
endif()
if(NOT DEFINED ROOT_SIZE)
    set(ROOT_SIZE        65536)
endif()

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
    -DHAVE_UART=${HAVE_UART}
    -DHAVE_GIO=${HAVE_GIO}
    -DROOT_OFFSET=${ROOT_OFFSET}
    -DROOT_SIZE=${ROOT_SIZE}
    -DPICO_SD_CLK_PIN=${PICO_SD_CLK_PIN}
    -DPICO_SD_CMD_PIN=${PICO_SD_CMD_PIN}
    -DPICO_SD_DAT0_PIN=${PICO_SD_DAT0_PIN}
    -DPICO_SD_DAT3_PIN=${PICO_SD_DAT3_PIN}
)
target_link_options(pfs_test PRIVATE -g)
target_link_libraries(pfs_test
    pico_stdlib
    pico_binary_info
    pico_status_led
    )

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
    pico_status_led
    pfs_dev_kbd
    pfs_dev_keymap_uk
    )

pico_add_extra_outputs(kbd_test)
pico_enable_stdio_usb(kbd_test 0)
pico_enable_stdio_uart(kbd_test 1)
