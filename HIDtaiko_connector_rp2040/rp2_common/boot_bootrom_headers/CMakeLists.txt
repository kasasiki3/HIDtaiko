add_library(boot_bootrom_headers INTERFACE)
target_include_directories(boot_bootrom_headers SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
