add_executable(${PROJECT_NAME}
    main.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/Lib
    ${CMAKE_SOURCE_DIR}/Core/Inc
)

# target_include_directories(${PROJECT_NAME} PUBLIC ../Lib)
# target_include_directories(${PROJECT_NAME} PUBLIC ../Lib)
target_link_libraries(${PROJECT_NAME} PUBLIC
    pico_stdlib
    pico_util
    pico_multicore
    pico_time

    hardware_i2c
    hardware_adc
    hardware_gpio
    hardware_rtc
    hardware_timer
    hardware_dma
    hardware_irq
    hardware_spi

    ssd1306
    physical-api
    fatfs
    ds3231
    BCD_Conversion
)

set_target_properties(${PROJECT_NAME} PROPERTIES

    # RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Generated"
    # OUTPUT_NAME ${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Generated_debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Generated_Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Generated_Release"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/Generated_MinSizeRel"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/Generated_RelWithDebInfo"

    OUTPUT_NAME_DEBUG "${PROJECT_NAME}_Debug_${PROJECT_VERSION}"
    OUTPUT_NAME_RELEASE "${PROJECT_NAME}_Release_${PROJECT_VERSION}"
    OUTPUT_NAME_MINSIZEREL "${PROJECT_NAME}_MinSizeRel_${PROJECT_VERSION}"
    OUTPUT_NAME_RELWITHDEBINFO "${PROJECT_NAME}_RelWithDebInfo_${PROJECT_VERSION}"
)

pico_add_extra_outputs(${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/Generated_${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.uf2"
    "${CMAKE_SOURCE_DIR}/Generated_${CMAKE_BUILD_TYPE}/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_${PROJECT_VERSION}.uf2"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
    "${CMAKE_SOURCE_DIR}/Generated_${CMAKE_BUILD_TYPE}/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_${PROJECT_VERSION}.hex"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
    "${CMAKE_SOURCE_DIR}/Generated_${CMAKE_BUILD_TYPE}/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_${PROJECT_VERSION}.bin"
    COMMENT "Copying all output files to Generated_${CMAKE_BUILD_TYPE}"
)

# First, create the memory report script
file(WRITE "${CMAKE_BINARY_DIR}/pico_memory_report.cmake"
    "# Read the target file path from the temp file
file(READ \"\${CMAKE_BINARY_DIR}/target_file_path.txt\" TARGET_FILE)
string(STRIP \"\${TARGET_FILE}\" TARGET_FILE)

if(NOT EXISTS \"\${TARGET_FILE}\")
    message(WARNING \"Target file not found: \${TARGET_FILE}\")
    return()
endif()

execute_process(
    COMMAND ${CMAKE_SIZE_UTIL} -A \"\${TARGET_FILE}\"
    OUTPUT_VARIABLE SIZE_OUTPUT
    RESULT_VARIABLE SIZE_RESULT
)

if(NOT SIZE_RESULT EQUAL 0)
    message(WARNING \"Failed to run size utility\")
    return()
endif()

# Initialize sizes
set(TEXT_SIZE 0)
set(DATA_SIZE 0)
set(BSS_SIZE 0)

# Parse the -A output format to extract section sizes
string(REGEX MATCH \".text[ ]+([0-9]+)\" TEXT_MATCH \"\${SIZE_OUTPUT}\")
string(REGEX MATCH \".data[ ]+([0-9]+)\" DATA_MATCH \"\${SIZE_OUTPUT}\")
string(REGEX MATCH \".bss[ ]+([0-9]+)\" BSS_MATCH \"\${SIZE_OUTPUT}\")

if(TEXT_MATCH)
    string(REGEX REPLACE \".text[ ]+([0-9]+)\" \"\\\\1\" TEXT_SIZE \"\${TEXT_MATCH}\")
endif()
if(DATA_MATCH)
    string(REGEX REPLACE \".data[ ]+([0-9]+)\" \"\\\\1\" DATA_SIZE \"\${DATA_MATCH}\")
endif()
if(BSS_MATCH)
    string(REGEX REPLACE \".bss[ ]+([0-9]+)\" \"\\\\1\" BSS_SIZE \"\${BSS_MATCH}\")
endif()

# Calculate flash and RAM usage
math(EXPR FLASH_USED \"\${TEXT_SIZE} + \${DATA_SIZE}\")
math(EXPR RAM_USED \"\${DATA_SIZE} + \${BSS_SIZE}\")

# RP2040 specs: 2MB flash (2097152 bytes), 264KB RAM (270336 bytes)
math(EXPR FLASH_PERCENT \"\${FLASH_USED} * 100 / 2097152\")
math(EXPR RAM_PERCENT \"\${RAM_USED} * 100 / 270336\")

message(\"Pico RP2040 Memory Usage:\")
message(\"  Flash: \${FLASH_USED} bytes / 2048 KB (\${FLASH_PERCENT}%)\")
message(\"  RAM:   \${RAM_USED} bytes / 264 KB (\${RAM_PERCENT}%)\")
message(\"  Sections: text=\${TEXT_SIZE} data=\${DATA_SIZE} bss=\${BSS_SIZE}\")
")

# Alternative: Use cmake to write the file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete! Output files in:"
    COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_SOURCE_DIR}/Generated_${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""

    # Show detailed section sizes
    COMMAND ${CMAKE_SIZE_UTIL} -A $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

    # Write the target file path using cmake
    COMMAND ${CMAKE_COMMAND} -DINPUT="$<TARGET_FILE:${PROJECT_NAME}>" -DOUTPUT="${CMAKE_BINARY_DIR}/target_file_path.txt" -P "${CMAKE_BINARY_DIR}/write_path.cmake"

    # Run the memory report script
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/pico_memory_report.cmake"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
)

# Create the helper script to write the path
file(WRITE "${CMAKE_BINARY_DIR}/write_path.cmake"
    "file(WRITE \"\${OUTPUT}\" \"\${INPUT}\")
")
