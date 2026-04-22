add_library(ssd1306
    ssd1306.c
)

target_link_libraries(ssd1306
    hardware_i2c
)

target_include_directories(ssd1306 PUBLIC "./")