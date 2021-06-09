set(name HomeUnit)
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(${name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(srcs
  main.cpp
  hd44780.c
  hd44780.h
  )

add_executable(${name} ${srcs})
pico_generate_pio_header(${name} ${CMAKE_CURRENT_SOURCE_DIR}/pio_hd44780.pio)

target_link_libraries(${name} pico_stdlib hardware_pio hardware_dma hardware_irq)

pico_add_extra_outputs(${name})

