cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

project(picow)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()
pico_sdk_init()


add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(picow
    picow.c
    usb_descriptors.c
    ${PICO_SDK_PATH}/lib/tinyusb/hw/bsp/rp2040/family.c
    )

# Add the TinyUSB include directory to the include path
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PICO_SDK_PATH}/lib/tinyusb/src
    ${PICO_SDK_PATH}/lib/tinyusb/src/tusb
    ${PICO_SDK_PATH}/lib/tinyusb/src/device)

target_compile_definitions(${PROJECT_NAME} PRIVATE CFG_TUSB_MCU=OPT_MCU_RP2040)

include_directories(${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi_pico)

add_compile_definitions(
    CFG_TUD_ENABLED=true
    CFG_TUD_HID=true
    CFG_TUSB_CONFIG_FILE="tusb_config.h"
)

pico_enable_stdio_usb(picow 1)
pico_enable_stdio_uart(picow 0)

target_link_libraries(picow
    pico_stdlib
    pico_cyw43_arch_none
    tinyusb_device
    tinyusb_board
    )

pico_add_extra_outputs(picow)
