cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/edge_model)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ov7670-fifo-camera)

set(EI_SDK_PATH "C:/ncs/v3.2.4/modules/edge-impulse-sdk-zephyr")

add_subdirectory(
    ${EI_SDK_PATH}/edge-impulse-sdk/cmake/zephyr
    edge-impulse-sdk-zephyr
)

target_sources(app PRIVATE
    src/main.cpp
    src/ov7670.c
    src/fifo.c
    src/tft_display.c
)