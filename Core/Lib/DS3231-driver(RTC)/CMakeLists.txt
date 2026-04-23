add_library(ds3231
    ds3231.c
)

target_link_libraries(ds3231
    hardware_i2c
)

target_include_directories(ds3231 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)