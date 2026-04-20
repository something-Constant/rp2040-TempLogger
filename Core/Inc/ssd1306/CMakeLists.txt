add_library(lib
    ssd1306.c
)

target_link_libraries(lib
    hardware_i2c
)

target_include_directories(lib PUBLIC "./")