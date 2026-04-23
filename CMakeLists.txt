cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW7_mcp4912_spi_dac C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW7_mcp4912_spi_dac
    HW7_main.c
)

target_link_libraries(HW7_mcp4912_spi_dac
    pico_stdlib
    hardware_spi
    hardware_gpio
    m
)

pico_add_extra_outputs(HW7_mcp4912_spi_dac)
