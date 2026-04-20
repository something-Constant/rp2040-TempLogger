cmake_minimum_required(VERSION 3.13)

# set(PICO_GCC_TRIPLE "arm-none-eabi")
# set(PICO_TOOLCHAIN_PATH "P:/13.2 Rel1/bin")
set(PICO_SDK_PATH,"P:/pico-sdk")
set(PICO_PLATFORM "rp2040")
set(PICO_BOARD "pico")
set(PICO_DEOPTIMIZED_DEBUG 1)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
set(CMAKE_GENERATOR "Ninja")
set(CMAKE_GENERATOR_PLATFORM Ninja)

# set(CMAKE_C_COMPILER arm-none-eabi-gcc.exe)
# set(CMAKE_CXX_COMPILER arm-none-eabi-g++.exe)
include(pico_sdk_import.cmake)

project(Button
    VERSION 0.0.0
    DESCRIPTION "C comport serial cominucation"
    C

    # LANGUAGES C
    # CMAKE_BUILD_TYPE Ninja
    # CMAKE_CONFIGURATION_TYPES Debug
)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# add_subdirectory(Driver)
add_subdirectory(Core)

# include_directories(${PROJECT_NAME}
# Driver/ssd1306
# Core/Inc
# )

# add_library(LCD
# Driver/ssd1306/ssd1306.c
# )

# add_executable(${PROJECT_NAME}
# main.c
# )

# target_link_libraries(${PROJECT_NAME}
# pico_stdlib
# hardware_i2c

# # LCD
# lib
# )

# pico_add_extra_outputs(${PROJECT_NAME})

# cmake -G Ninja ..