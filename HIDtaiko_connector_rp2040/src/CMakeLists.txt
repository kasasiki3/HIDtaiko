add_executable(keyboard)

target_sources(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_include_directories(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(keyboard PUBLIC
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_adc
    hardware_flash
    hardware_sync)

pico_add_extra_outputs(keyboard)
pico_enable_stdio_uart(keyboard 0)
pico_enable_stdio_usb(keyboard 1)
