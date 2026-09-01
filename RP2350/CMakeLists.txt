cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)
set(PICO_BOARD waveshare_rp2350_touch_lcd_4.3b CACHE STRING "Target board")

include(cmake/pico_sdk_import.cmake)
project(coinflip_rp2350 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR "Pico SDK 2.2.0 or newer is required")
endif()

pico_sdk_init()

# Required by the Waveshare board for reliable cold starts.
add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

add_library(coinflip_core STATIC
    ../common/bip39_lookup.c
    ../common/mnemonic_state.c
    ../common/sha256.c
)
target_include_directories(coinflip_core PUBLIC include ../common)

add_library(waveshare_psram STATIC
    vendor/waveshare/psram/rp_pico_alloc.c
    vendor/waveshare/psram/tlsf.c
)
target_include_directories(waveshare_psram PUBLIC vendor/waveshare/psram)
target_link_libraries(waveshare_psram PUBLIC
    pico_stdlib hardware_exception hardware_flash hardware_gpio hardware_spi
    hardware_sync pico_flash)

add_library(waveshare_display STATIC
    vendor/waveshare/bsp/bsp_dma_channel_irq.c
    vendor/waveshare/bsp/bsp_st7262.c
    vendor/waveshare/bsp/pio_rgb.c)
target_include_directories(waveshare_display PUBLIC vendor/waveshare/bsp)
target_link_libraries(waveshare_display PUBLIC
    pico_stdlib hardware_dma hardware_irq hardware_pio hardware_pwm)
pico_generate_pio_header(waveshare_display
    ${CMAKE_CURRENT_LIST_DIR}/vendor/waveshare/bsp/pio_rgb.pio)

add_library(waveshare_touch STATIC
    vendor/waveshare/bsp/bsp_gt911.c
    vendor/waveshare/bsp/bsp_i2c.c)
target_include_directories(waveshare_touch PUBLIC vendor/waveshare/bsp)
target_link_libraries(waveshare_touch PUBLIC
    pico_stdlib hardware_i2c hardware_irq)

add_library(coinflip_fonts STATIC
    vendor/waveshare/fonts/font16.c
    vendor/waveshare/fonts/font20.c
    vendor/waveshare/fonts/font24.c)
target_include_directories(coinflip_fonts PUBLIC vendor/waveshare/fonts)

add_library(coinflip_graphics STATIC
    src/ui/graphics.c
    src/ui/coinflip_ui.c
    ../common/utf8.c)
target_include_directories(coinflip_graphics PUBLIC include ../common)
target_link_libraries(coinflip_graphics PUBLIC coinflip_fonts)

add_executable(coinflip_rp2350
    src/main.c
    src/app/coinflip_app.c
    src/platform/rp2350/clock.c)
target_include_directories(coinflip_rp2350 PRIVATE include)
target_link_libraries(coinflip_rp2350 PRIVATE
    coinflip_core coinflip_graphics waveshare_display waveshare_touch
    waveshare_psram pico_stdlib hardware_clocks)
target_compile_definitions(coinflip_rp2350 PRIVATE PICO_EMBED_XIP_SETUP=1)
pico_enable_stdio_usb(coinflip_rp2350 0)
pico_enable_stdio_uart(coinflip_rp2350 0)
pico_add_extra_outputs(coinflip_rp2350)
