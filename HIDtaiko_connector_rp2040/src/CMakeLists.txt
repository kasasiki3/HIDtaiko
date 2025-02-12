cmake_minimum_required(VERSION 3.13)
cmake_minimum_required(VERSION 3.13)
# Pico SDKのパスをインクルード
include(C:/Users/kasashiki/Documents/HIDtaiko.picover/HIDtaiko.ver3.0/pico_sdk_import.cmake)
# Pico SDKの初期化
pico_sdk_init()
add_executable(keyboard)

target_sources(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
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

