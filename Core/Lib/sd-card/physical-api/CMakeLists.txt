add_library(physical-api
    ./fatfs_sd.c
)

# target_include_directories(physical-api PUBLIC
# ${CMAKE_CURRENT_SOURCE_DIR}/inc  # Use absolute path
# ${CMAKE_CURRENT_SOURCE_DIR}      # For ./
# ${CMAKE_SOURCE_DIR}/Core/Inc     # Where main.h likely lives
# )
target_include_directories(physical-api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # For ./
    ${CMAKE_SOURCE_DIR}/Core/Inc # Where main.h likely lives
)

target_link_libraries(physical-api
    hardware_spi
    hardware_gpio
    pico_time

    fatfs
)
