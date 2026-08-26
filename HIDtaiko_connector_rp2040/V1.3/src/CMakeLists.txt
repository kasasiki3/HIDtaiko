add_executable(keyboard)

target_sources(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_include_directories(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

pico_generate_pio_header(keyboard ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_link_libraries(keyboard PUBLIC
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_adc
    hardware_flash
    hardware_pio
    hardware_sync
    pico_unique_id)

# 切り分け用: cmake -DFORCE_SWITCH_MODE=1 で SW1 を無視してSwitchモード固定にする
if(FORCE_SWITCH_MODE)
    target_compile_definitions(keyboard PRIVATE FORCE_SWITCH_MODE=1)
endif()

pico_add_extra_outputs(keyboard)
pico_enable_stdio_uart(keyboard 0)
pico_enable_stdio_usb(keyboard 1)
