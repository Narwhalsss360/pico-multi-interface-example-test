cmake_minimum_required(VERSION 3.13...3.27)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(pico_sdk_import.cmake)

project(multi_interface)
pico_sdk_init()

add_executable(multi_interface)
target_sources(
    multi_interface
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_include_directories(
    multi_interface
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(
    multi_interface
    PUBLIC
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(multi_interface 1)
pico_add_extra_outputs(multi_interface)
