if (NOT TARGET pico_cxx_options)
    pico_add_library(pico_cxx_options)

    target_sources(pico_cxx_options INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/new_delete.cpp
    )

    # PICO_CMAKE_CONFIG: PICO_CXX_ENABLE_EXCEPTIONS, Enable CXX exception handling, type=bool, default=0, group=pico_cxx_options
    # PICO_BUILD_DEFINE: PICO_CXX_ENABLE_EXCEPTIONS, value of CMake var PICO_CXX_ENABLE_EXCEPTIONS, type=string, default=0, group=pico_cxx_options
    if (NOT PICO_CXX_ENABLE_EXCEPTIONS)
        target_compile_definitions( pico_cxx_options INTERFACE PICO_CXX_ENABLE_EXCEPTIONS=0)
        target_compile_options( pico_cxx_options INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
        target_compile_options( pico_cxx_options INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-unwind-tables>)
    else()
        target_compile_definitions( pico_cxx_options INTERFACE PICO_CXX_ENABLE_EXCEPTIONS=1)
    endif()

    # PICO_CMAKE_CONFIG: PICO_CXX_ENABLE_RTTI, Enable CXX rtti, type=bool, default=0, group=pico_cxx_options
    if (NOT PICO_CXX_ENABLE_RTTI)
        target_compile_options( pico_cxx_options INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
    endif()

    # PICO_CMAKE_CONFIG: PICO_CXX_ENABLE_CXA_ATEXIT, Enable cxa-atexit, type=bool, default=0, group=pico_cxx_options
    if (NOT PICO_CXX_ENABLE_CXA_ATEXIT)
        target_compile_options( pico_cxx_options INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
    endif()
endif()
