add_executable(${PROJECT_NAME}
    main.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/Lib
    ${CMAKE_SOURCE_DIR}/Core/Inc
)


# target_include_directories(${PROJECT_NAME} PUBLIC ../Lib)
# target_include_directories(${PROJECT_NAME} PUBLIC ../Lib)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_util
    pico_multicore

    hardware_i2c
    hardware_adc
    hardware_gpio
    hardware_rtc
    hardware_timer
    hardware_dma
    hardware_irq
    hardware_spi

    ssd1306
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete! Output files in:"
    COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_SOURCE_DIR}/Generated_${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
)
