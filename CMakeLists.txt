cmake_minimum_required(VERSION 3.13)

# Enable developer warnings for this project
set(CMAKE_WARN_DEV_WARNINGS ON)

# Enable deprecated warnings
set(CMAKE_WARN_DEPRECATED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CXX_COMPILE_OBJECT)
    set(CMAKE_CXX_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
    set(CMAKE_CXX_COMPILER "${CMAKE_C_COMPILER}")
endif()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Generated_Files)
set(PICO_GCC_TRIPLE "arm-none-eabi")
set(PICO_TOOLCHAIN_PATH "P:/13.2 Rel1/bin")
set(PICO_SDK_PATH "P:/pico-sdk")
set(PICO_PLATFORM "rp2040")
set(PICO_BOARD "pico")
set(PICO_DEOPTIMIZED_DEBUG 1)


# Build Type
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)

# set(CMAKE_BUILD_TYPE "Debug")       # No optimization, debug symbols
# set(CMAKE_BUILD_TYPE "Release")     # Full optimization, no debug
# set(CMAKE_BUILD_TYPE "RelWithDebInfo")  # Optimized + debug symbols
# set(CMAKE_BUILD_TYPE "MinSizeRel")  # Optimize for size
include(pico_sdk_import.cmake)

project(Temp_Logger
    VERSION 0.0.0
    DESCRIPTION "Temperture Logger"
    LANGUAGES C CXX
)



# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
# add_subdirectory(Driver)
add_subdirectory(Core)


